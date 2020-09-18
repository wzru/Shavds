package main

import(
	"net/http"
	"io"
	"fmt"
)

func Pong(w http.ResponseWriter, r *http.Request) {
	fmt.Println("ping!")
	io.WriteString(w, "pong!")
}

func Draw(w http.ResponseWriter, r *http.Request) {
	
}

func CmpFun(w http.ResponseWriter, r *http.Request) {
	
}

func CmpCfg(w http.ResponseWriter, r *http.Request) {
	
}

func main() {
	http.HandleFunc("/ping", Pong)
	http.HandleFunc("/draw", Draw)
	http.HandleFunc("/cmpfun", CmpFun)
	http.HandleFunc("/cmpcfg", CmpCfg)
	http.ListenAndServe(":7000", nil)
}