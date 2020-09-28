package main

import (
	"bufio"
	"crypto/md5"
	"io"
	"net/http"
	"os/exec"
	"regexp"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
)

// 颜色预定义
var RED, GREEN, YELLEW, BLUE, PINK, RES = "\x1b\\[1;41m", "\x1b\\[1;42m", "\x1b\\[43;37m", "\x1b\\[1;44m", "\x1b\\[1;45m", "\x1b\\[0m"
var progress = make(map[[16]byte]float64)
var llReg = regexp.MustCompile(GREEN + "successfully" + RES + " generated '" + BLUE + "(.*?)" + RES)
var smReg = regexp.MustCompile("'(.*?)' '(.*?)' (.*?) (.*?) (.*?) (.*?) '(.*?)' '(.*?)' (.*)")

type cmpRes struct {
	Func1, Func2, File1, File2 string
	Cnt1, Cnt2, Line1, Line2   int
	Sim                        float64
}

func pong(c *gin.Context) {
	c.String(http.StatusOK, "pong!")
}

func draw(c *gin.Context) {
	file := c.Query("file")
	tp := c.Query("type")
	cmd1 := exec.Command("./core/obfuscate-merge.sh", "-O0", "-g", file)
	out1, _ := cmd1.Output()
	reg := regexp.MustCompile(`successfully generated \'(?s:(.*?))\'`)
	res := (reg.FindAllStringSubmatch(string(out1), -1))
	ll := ""
	for _, text := range res {
		ll = text[1]
	}
	cmd2 := exec.Command("./core/draw.sh", tp, ll)
	out2, _ := cmd2.Output()
	res = reg.FindAllStringSubmatch(string(out2), -1)
	img := ""
	for _, text := range res {
		img = text[1]
	}
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    img,
	})
}

func cmpfun(c *gin.Context) {
	file1 := c.Query("file1")
	file2 := c.Query("file2")
	h := hash(file1, file2)
	data := []cmpRes{}
	progress[h] = 0
	cmd1 := exec.Command("./core/obfuscate-merge.sh", "-O3", "-g", file1, file2)
	out1, _ := cmd1.Output()
	res := llReg.FindStringSubmatch(string(out1))
	ll := res[1]
	cmd2 := exec.Command("./core/shavds.sh", "cmpfun", ll)
	buf := make([]byte, 256)
	stderr, _ := cmd2.StderrPipe()
	cmd2.Start()
	reader := bufio.NewReader(stderr)
	for {
		cnt, err := reader.Read(buf)
		if err != nil || io.EOF == err {
			break
		}
		for _, line := range strings.Split(strings.Replace(string(buf[0:cnt]), "\r", "", -1), "\n") {
			if strings.HasPrefix(line, "progress ") {
				prog, _ := strconv.ParseFloat(strings.Split(line, " ")[1], 64)
				progress[h] = prog
			} else if strings.HasPrefix(line, "'") {
				res := smReg.FindStringSubmatch(line)
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
	file1 := c.Query("file1")
	file2 := c.Query("file2")
	h := hash(file1, file2)
	data := []cmpRes{}
	progress[h] = 0
	cmd1 := exec.Command("./core/obfuscate-merge.sh", "-O3", "-g", file1, file2)
	out1, _ := cmd1.Output()
	// fmt.Printf("out1=%v\n", string(out1))
	// fmt.Printf("out1=%v\n", out1)
	res := llReg.FindStringSubmatch(string(out1))
	// fmt.Printf("res[%d]=%v\n", len(res), res)
	ll := res[1]
	// fmt.Printf("ll=%v\nreg=%v\n", ll, llReg.String())
	// fmt.Printf("reg=%v\n", reg.String())
	cmd2 := exec.Command("./core/shavds.sh", "cmpcfg", ll)
	buf := make([]byte, 256)
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
		for _, line := range strings.Split(strings.Replace(string(buf[0:cnt]), "\r", "", -1), "\n") {
			// fmt.Printf("line:%v\n", line)
			if strings.HasPrefix(line, "progress ") {
				prog, _ := strconv.ParseFloat(strings.Split(line, " ")[1], 64)
				// fmt.Printf("orig=%v\nprogress=%v\n", strings.Split(line, " ")[1], prog)
				progress[h] = prog
			} else if strings.HasPrefix(line, "'") {
				res := smReg.FindStringSubmatch(line)
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
	file1 := c.Query("file1")
	file2 := c.Query("file2")
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    progress[hash(file1, file2)]})
}

func main() {
	// cmd1 := exec.Command("pwd")
	// out1, _ := cmd1.Output()
	// fmt.Println(string(out1))
	r := gin.Default()
	r.GET("/ping", pong)
	r.GET("/draw", draw)
	r.GET("/cmpfun", cmpfun)
	r.GET("/cmpcfg", cmpcfg)
	r.GET("/progress", getProgress)
	r.Run("0.0.0.0:7000")
}
