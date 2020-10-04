package main

import (
	"bufio"
	"crypto/md5"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/dgrijalva/jwt-go"
	"github.com/gin-gonic/gin"
)

// 颜色预定义
var RED, GREEN, YELLEW, BLUE, PINK, RES = "\x1b\\[1;41m", "\x1b\\[1;42m", "\x1b\\[43;37m", "\x1b\\[1;44m", "\x1b\\[1;45m", "\x1b\\[0m"
var progress = make(map[[16]byte]float64)
var cllReg = regexp.MustCompile(GREEN + "successfully" + RES + " generated '" + BLUE + "(.*)" + RES)
var llReg = regexp.MustCompile("successfully generated '(.*)'")
var cmpReg = regexp.MustCompile("'(.*?)' '(.*?)' (.*?) (.*?) (.*?) (.*?) '(.*?)' '(.*?)' (.*)")
var dataDir = "./data/"
var codeExts = []string{".cpp", ".c", ".go", ".cc", ".cxx"}

type result struct {
	Success bool        `json:"success"`
	Data    interface{} `json:"data"`
	Msg     string      `json:"msg"`
	Code    int         `json:"code"`
}

type cmpRes struct {
	Func1 string  `json:"func1"`
	Func2 string  `json:"func2"`
	File1 string  `json:"file1"`
	File2 string  `json:"file2"`
	Cnt1  int     `json:"inst1"`
	Cnt2  int     `json:"inst2"`
	Line1 int     `json:"line1"`
	Line2 int     `json:"line2"`
	Sim   float64 `json:"similarity"`
}

type vulRes struct {
	Type string `json:"type"`
	Line int    `json:"line"`
	Col  int    `json:"column"`
}

func cors() gin.HandlerFunc {
	return func(c *gin.Context) {
		origin := c.Request.Header.Get("origin")
		if len(origin) == 0 {
			origin = c.Request.Header.Get("Origin")
		}
		c.Writer.Header().Set("Access-Control-Allow-Origin", origin)
		c.Writer.Header().Set("Access-Control-Allow-Credentials", "true")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization, accept, origin, Cache-Control, X-Requested-With")
		c.Writer.Header().Set("Access-Control-Allow-Methods", "OPTIONS, GET, POST, DELETE")
		c.Writer.Header().Set("Content-Type", "application/json; charset=utf-8")
		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}
		c.Next()
	}
}

func printHTTP(c *gin.Context) {
	body, _ := ioutil.ReadAll(c.Request.Body)
	fmt.Println("---body/--- \r\n " + string(body))
	fmt.Println("---header/--- \r\n")
	for k, v := range c.Request.Header {
		fmt.Println(k, v)
	}
	fmt.Printf("ip=%v\nua=%v\n", c.ClientIP, c.Request.Header["User-Agent"])
}

func pong(c *gin.Context) {
	printHTTP(c)
	c.String(http.StatusOK, "pong!")
}

func draw(c *gin.Context) {
	cookie, _ := c.Cookie("shavds")
	if cookie == "" {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"data":    nil,
			"msg":     "缺少cookie",
		})
		return
	}
	dir := dataDir + cookie + "/"
	file := c.Query("file")
	tp := c.Query("type")
	cmd1 := exec.Command("./core/gen.sh", "-O0", "-g", dir+file)
	fmt.Println(cmd1.String())
	out1, _ := cmd1.Output()
	reg := regexp.MustCompile(`successfully generated \'(?s:(.*?))\'`)
	res := (reg.FindAllStringSubmatch(string(out1), -1))
	ll := ""
	for _, text := range res {
		ll = text[1]
	}
	cmd2 := exec.Command("./core/draw.sh", "-T", tp, ll)
	fmt.Println(cmd2.String())
	out2, _ := cmd2.Output()
	res = reg.FindAllStringSubmatch(string(out2), -1)
	imgs := []string{}
	// fmt.Printf("res=%v\n", res)
	for _, text := range res {
		// fmt.Printf("text=%v\n", text)
		imgs = append(imgs, filepath.Base(text[1]))
	}
	fmt.Printf("imgs=%v\n", imgs)
	// c.File(img)
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"images": imgs,
		}})
}

func cmpfun(c *gin.Context) {
	cookie, _ := c.Cookie("shavds")
	if cookie == "" {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"data":    nil,
			"msg":     "缺少cookie",
		})
		return
	}
	dir := dataDir + cookie + "/"
	file1 := dir + c.Query("file1")
	file2 := dir + c.Query("file2")
	h := hash(file1, file2)
	data := []cmpRes{}
	progress[h] = 0
	cmd1 := exec.Command("./core/gen.sh", "-O3", "-g", file1, file2)
	fmt.Println(cmd1.String())
	out1, _ := cmd1.Output()
	res := cllReg.FindStringSubmatch(string(out1))
	ll := res[1]
	// fmt.Printf("ll=%v\n", ll)
	cmd2 := exec.Command("./core/shavds.sh", "cmpfun", ll)
	fmt.Println(cmd2.String())
	buf := make([]byte, 1024)
	stderr, _ := cmd2.StderrPipe()
	cmd2.Start()
	reader := bufio.NewReader(stderr)
	for {
		cnt, err := reader.Read(buf)
		if err != nil || io.EOF == err {
			break
		}
		for _, line := range strings.Split(strings.Replace(string(buf[0:cnt]), "\r", "\n", -1), "\n") {
			if strings.HasPrefix(line, "progress ") {
				prog, _ := strconv.ParseFloat(strings.Split(line, " ")[1], 64)
				progress[h] = prog
			} else if strings.HasPrefix(line, "'") {
				res := cmpReg.FindStringSubmatch(line)
				// fmt.Printf("res=%v\n", res)
				if len(res) == 0 {
					continue
				}
				cnt1, _ := strconv.Atoi(res[3])
				cnt2, _ := strconv.Atoi(res[4])
				line1, _ := strconv.Atoi(res[5])
				line2, _ := strconv.Atoi(res[6])
				sim, _ := strconv.ParseFloat(res[9], 64)
				cmpRes := cmpRes{
					Func1: res[1],
					Func2: res[2],
					Cnt1:  cnt1,
					Cnt2:  cnt2,
					Line1: line1,
					Line2: line2,
					File1: res[7],
					File2: res[8],
					Sim:   sim,
				}
				data = append(data, cmpRes)
			}
		}
	}
	cmd2.Wait()
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    data,
	})
}

func cmpcfg(c *gin.Context) {
	cookie, _ := c.Cookie("shavds")
	if cookie == "" {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"data":    nil,
			"msg":     "缺少cookie",
		})
		return
	}
	dir := dataDir + cookie + "/"
	file1 := dir + c.Query("file1")
	file2 := dir + c.Query("file2")
	h := hash(file1, file2)
	// fmt.Printf("hash=%v\n", h)
	data := []cmpRes{}
	progress[h] = 0
	cmd1 := exec.Command("./core/gen.sh", "-O3", "-g", file1, file2)
	fmt.Println(cmd1.String())
	out1, _ := cmd1.Output()
	// fmt.Printf("out1=%v\n", string(out1))
	// fmt.Printf("out1=%v\n", out1)
	res := cllReg.FindStringSubmatch(string(out1))
	// fmt.Printf("res[%d]=%v\n", len(res), res)
	ll := res[1]
	// fmt.Printf("ll=%v\nreg=%v\n", ll, cllReg.String())
	// fmt.Printf("reg=%v\n", reg.String())
	cmd2 := exec.Command("./core/shavds.sh", "cmpcfg", ll)
	fmt.Println(cmd2.String())
	buf := make([]byte, 1024)
	stderr, _ := cmd2.StderrPipe()
	cmd2.Start()
	reader := bufio.NewReader(stderr)
	for {
		cnt, err := reader.Read(buf)
		if err != nil || io.EOF == err {
			break
		}
		// fmt.Println(cnt)
		// fmt.Println(string(buf[0:cnt]))
		for _, line := range strings.Split(strings.Replace(string(buf[0:cnt]), "\r", "\n", -1), "\n") {
			// fmt.Printf("line:%v\n", line)
			if strings.HasPrefix(line, "progress ") {
				prog, _ := strconv.ParseFloat(strings.Split(line, " ")[1], 64)
				// fmt.Printf("[]byte(line)=%v\nline=%v\nprogress=%v\n", []byte(line), line, prog)
				progress[h] = prog
			} else if strings.HasPrefix(line, "'") {
				res := cmpReg.FindStringSubmatch(line)
				if len(res) == 0 {
					continue
				}
				cnt1, _ := strconv.Atoi(res[3])
				cnt2, _ := strconv.Atoi(res[4])
				line1, _ := strconv.Atoi(res[5])
				line2, _ := strconv.Atoi(res[6])
				sim, _ := strconv.ParseFloat(res[9], 64)
				cmpRes := cmpRes{
					Func1: res[1],
					Func2: res[2],
					Cnt1:  cnt1,
					Cnt2:  cnt2,
					Line1: line1,
					Line2: line2,
					File1: res[7],
					File2: res[8],
					Sim:   sim,
				}
				data = append(data, cmpRes)
				// fmt.Printf("res[%d]=%v\n", len(res), res)
				// fmt.Printf("cmpRes=%v\n", cmpRes)
				// fmt.Printf("data=%v\n", data)
			}
		}
	}
	cmd2.Wait()
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    data,
	})
}

func hash(s1 string, s2 string) [16]byte {
	if s1 > s2 {
		s1, s2 = s2, s1
	}
	return md5.Sum([]byte(s1 + s2))
}

func getProgress(c *gin.Context) {
	cookie, _ := c.Cookie("shavds")
	if cookie == "" {
		c.JSON(http.StatusUnauthorized, gin.H{
			"success": false,
			"data":    nil,
			"msg":     "缺少cookie",
		})
		return
	}
	dir := dataDir + cookie + "/"
	file1 := dir + c.Query("file1")
	file2 := dir + c.Query("file2")
	// fmt.Printf("hash=%v\n", hash(file1, file2))
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    progress[hash(file1, file2)]})
}

func genCookie(c *gin.Context) string {
	ip := c.ClientIP()
	ua := c.Request.Header["User-Agent"][0]
	t := time.Now().UnixNano()
	x := ip + ua + string(t)
	return fmt.Sprintf("%x", md5.Sum([]byte(x)))
}

// curl example:
// curl -F "files=@./func1.cpp" -F "files=@./func2.cpp"  http://localhost:7000/upload
func upload(c *gin.Context) {
	var clm *jwt.StandardClaims
	var err error
	token := ""
	auth := c.Request.Header.Get("Authorization")
	if len(auth) == 0 {
		token = genJWT(c)
		// fmt.Printf("token=%v\n", token)
		clm, _ = parseToken(token)
	} else {
		auth = strings.Fields(auth)[1]
		clm, err = parseToken(auth)
		if err != nil {
			token = genJWT(c)
			clm, _ = parseToken(strings.Fields(token)[1])
		}
	}
	name := clm.Id
	// fmt.Printf("name=%v\n", name)
	os.Mkdir(dataDir+name, os.ModePerm)
	form, err := c.MultipartForm()
	if err != nil {
		fmt.Println("获取多个文件出错", err)
		c.JSON(http.StatusOK, gin.H{
			"success": false,
			"data":    0,
			"msg":     err,
		})
		return
	}
	files := form.File["files"]
	fmt.Println(files)
	for _, file := range files {
		path := file.Filename
		err := c.SaveUploadedFile(file, `./data/`+name+`/`+path)
		if err != nil {
			fmt.Println("保存出错", err)
			c.JSON(http.StatusOK, gin.H{
				"success": false,
				"data":    0,
				"msg":     err,
			})
			return
		}
		fmt.Printf("get %s %d\n", path, file.Size)
	}
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    token,
	})
}

func getFile(c *gin.Context) {
	auth := strings.Fields(c.Request.Header.Get("Authorization"))[1]
	clm, _ := parseToken(auth)
	name := clm.Id
	file := dataDir + name + "/" + c.Param("file")
	c.File(file)
}

func delFile(c *gin.Context) {
	auth := strings.Fields(c.Request.Header.Get("Authorization"))[1]
	clm, _ := parseToken(auth)
	name := clm.Id
	file := dataDir + name + "/" + c.Param("file")
	os.Remove(file)
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    nil,
	})
}

func delFiles(c *gin.Context) {
	auth := strings.Fields(c.Request.Header.Get("Authorization"))[1]
	clm, _ := parseToken(auth)
	name := clm.Id
	os.RemoveAll(dataDir + name + "/")
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    nil,
	})
}

func isCodeExt(file string) bool {
	ext := path.Ext(file)
	for _, e := range codeExts {
		if e == ext {
			return true
		}
	}
	return false
}

func getList(c *gin.Context) {
	auth := strings.Fields(c.Request.Header.Get("Authorization"))[1]
	clm, _ := parseToken(auth)
	name := clm.Id
	dir := dataDir + name + "/"
	rd, _ := ioutil.ReadDir(dir)
	var files []string
	for _, file := range rd {
		if !file.IsDir() && isCodeExt(file.Name()) {
			files = append(files, file.Name())
		}
	}
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data": gin.H{
			"list": files,
		}})
}

func detect(c *gin.Context) {
	auth := strings.Fields(c.Request.Header.Get("Authorization"))[1]
	clm, _ := parseToken(auth)
	name := clm.Id
	dir := dataDir + name + "/"
	file := dir + c.Query("file")
	cmd1 := exec.Command("./core/gen.sh", "-O0", "-g", file)
	out1, _ := cmd1.Output()
	// fmt.Printf("out1=%v\n", string(out1))
	res := llReg.FindStringSubmatch(string(out1))
	ll := res[1]
	// fmt.Printf("res=%v\n", res)
	cmd2 := exec.Command("./core/shavds.sh", "detect", ll)
	buf := make([]byte, 1024)
	data := []vulRes{}
	stderr, _ := cmd2.StderrPipe()
	cmd2.Start()
	reader := bufio.NewReader(stderr)
	for {
		cnt, err := reader.Read(buf)
		if err != nil || io.EOF == err {
			break
		}
		for _, line := range strings.Split(string(buf[0:cnt]), "\n") {
			if len(line) == 0 {
				continue
			}
			res := strings.Split(line, " ")
			line, _ := strconv.Atoi(res[1])
			col, _ := strconv.Atoi(res[2])
			vulRes := vulRes{
				Type: res[0],
				Line: line,
				Col:  col,
			}
			data = append(data, vulRes)
		}
	}
	cmd2.Wait()
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    data,
	})
}

func main() {
	os.Mkdir(dataDir, os.ModePerm)
	r := gin.Default()
	r.Use(cors())
	r.GET("/ping", pong)
	r.POST("/upload", upload)
	r.Group("", authJWT())
	{
		r.GET("/list", getList)
		r.GET("/file/:file", getFile)
		r.DELETE("/file/:file", delFile)
		r.DELETE("/files", delFiles)
		r.POST("/draw", draw)
		r.GET("/cmpfun", cmpfun)
		r.GET("/cmpcfg", cmpcfg)
		r.GET("/progress", getProgress)
		r.GET("/detect", detect)
	}
	r.Run(":7000")
}
