package main

import (
	"bufio"
	"fmt"
	"io"
	"net/http"
	"os/exec"
	"regexp"

	"github.com/gin-gonic/gin"
)

var RED, GREEN, YELLEW, BLUE, PINK, RES = "\x1b\\[1;41m", "\x1b\\[1;42m", "\x1b\\[43;37m", "\x1b\\[1;44m", "\x1b\\[1;45m", "\x1b\\[0m"

func Pong(c *gin.Context) {
	c.String(http.StatusOK, "Pong!")
}

func Draw(c *gin.Context) {
	file := c.Query("file")
	tp := c.Query("type")
	cmd1 := exec.Command("./core/obfuscate-merge.sh", "-O0", file)
	out1, _ := cmd1.Output()
	reg := regexp.MustCompile(`successfully generated \'(?s:(.*?))\'`)
	res := (reg.FindAllStringSubmatch(string(out1), -1))
	ll := ""
	for _, text := range res {
		// fmt.Println("text=", text[1])
		ll = text[1]
	}
	// fmt.Printf("cmd1=%v\n", "./core/obfuscate-merge.sh -O0 "+file)
	// fmt.Printf("out1=%v\n", string(out1))
	// fmt.Printf("type=%v\n", tp)
	// fmt.Printf("file=%v\n", file)
	cmd2 := exec.Command("./core/draw.sh", tp, ll)
	out2, _ := cmd2.Output()
	res = reg.FindAllStringSubmatch(string(out2), -1)
	img := ""
	for _, text := range res {
		// fmt.Println("img=", text[1])
		img = text[1]
	}
	// fmt.Printf("out2=%v\n", string(out2))
	// fmt.Printf("img=%v\n", img)
	c.JSON(http.StatusOK, gin.H{
		"success": true,
		"data":    img,
	})
}

func CmpFun(c *gin.Context) {
	file1 := c.Query("file1")
	file2 := c.Query("file2")
	cmd1 := exec.Command("./core/obfuscate-merge.sh", "-O3", file1, file2)
	out1, _ := cmd1.Output()
	fmt.Printf("out1=%v\n", string(out1))
	fmt.Printf("out1=%v\n", out1)
	reg := regexp.MustCompile(GREEN + `successfully` + RES + ` generated '` + BLUE + `(?s:(.*?))` + RES)
	res := reg.FindAllStringSubmatch(string(out1), -1)
	ll := ""
	for _, text := range res {
		fmt.Printf("text type=%T\n", text)
		ll = text[1]
	}
	// fmt.Printf("ll=%v\nreg=%v\n", ll, reg.String())
	cmd2 := exec.Command("./core/shavds.sh", "cmpfun", ll)
	buf := make([]byte, 256)
	stderr, _ := cmd2.StderrPipe()
	cmd2.Start()
	reader := bufio.NewReader(stderr)
	for {
		line, err2 := reader.Read(buf)
		if err2 != nil || io.EOF == err2 {
			break
		}
		fmt.Println(line)
		fmt.Println(string(buf[0:line]))
	}
	cmd2.Wait()
}

func CmpCfg(c *gin.Context) {

}

func GetProgress(c *gin.Context) {

}

func main() {
	// cmd1 := exec.Command("pwd")
	// out1, _ := cmd1.Output()
	// fmt.Println(string(out1))
	r := gin.Default()
	r.GET("/ping", Pong)
	r.GET("/draw", Draw)
	r.GET("/cmpfun", CmpFun)
	r.GET("/cmpcfg", CmpCfg)
	r.GET("/progress", GetProgress)
	r.Run("0.0.0.0:7000")
}
