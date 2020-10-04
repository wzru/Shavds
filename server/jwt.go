package main

import (
	"net/http"
	"strings"
	"time"

	"github.com/dgrijalva/jwt-go"
	"github.com/gin-gonic/gin"
)

var secret = "50ftware5ecur1tyC0ur5ePr0ject2o2o"

func authJWT() gin.HandlerFunc {
	return func(ctx *gin.Context) {
		result := result{
			Success: false,
			// Code:    http.StatusUnauthorized,
			Msg:  "Unauthorized",
			Data: nil,
		}
		auth := ctx.Request.Header.Get("Authorization")
		// fmt.Printf("auth jwt=%v\n", auth)
		if len(auth) == 0 {
			// fmt.Printf("auth jwt2=%v\n", auth)
			ctx.JSON(http.StatusUnauthorized, result)
			ctx.Abort()
			return
		}
		auth = strings.Fields(auth)[1]
		_, err := parseToken(auth) // 校验token
		if err != nil {
			ctx.JSON(http.StatusUnauthorized, result)
			ctx.Abort()
			return
		}
		ctx.Next()
	}
}

func genJWT(c *gin.Context) string {
	now := time.Now().Unix()
	claims := jwt.StandardClaims{
		Audience:  genCookie(c),             // 受众
		ExpiresAt: now + int64(3600*24*365), // 失效时间
		Id:        genCookie(c),             // 编号
		IssuedAt:  now,                      // 签发时间
		Issuer:    "Shavds",                 // 签发人
		NotBefore: now,                      // 生效时间
		Subject:   "Shavds",                 // 主题
	}
	// fmt.Printf("id=%v\n", claims.Id)
	tokenClaims := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	token, _ := tokenClaims.SignedString([]byte(secret))
	// fmt.Printf("genToken=%v\n", token)
	return token
}

func parseToken(token string) (*jwt.StandardClaims, error) {
	jwtToken, err := jwt.ParseWithClaims(token, &jwt.StandardClaims{}, func(token *jwt.Token) (i interface{}, e error) {
		return []byte(secret), nil
	})
	if err == nil && jwtToken != nil {
		if claim, ok := jwtToken.Claims.(*jwt.StandardClaims); ok && jwtToken.Valid {
			return claim, nil
		}
	}
	return nil, err
}
