package main

import (
	"net/http"
	"os/exec"
	"regexp"

	"github.com/gin-gonic/gin"
)

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

}

func CmpCfg(c *gin.Context) {

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
	r.Run("0.0.0.0:7000")
}
