## Ping

#### 接口URL
> http://{{host}}/ping

#### 请求方式
> GET

#### Content-Type
> form-data







#### 成功响应示例
```javascript
Pong!
```



## 上传文件

#### 接口URL
> http://{{host}}/upload

#### 请求方式
> POST

#### Content-Type
> form-data






#### 请求Body参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| files     | C:\fakepath\func1.cpp |  必填 | - |
| files     | C:\fakepath\func2.cpp |  必填 | - |
| files     | C:\fakepath\stack.cpp |  必填 | - |

#### 成功响应示例
```javascript
{
	"data": 2,
	"success": true
}
```



## 获取文件

#### 接口URL
> http://{{host}}/file/main-func1-cfg.png

#### 请求方式
> GET

#### Content-Type
> form-data









## 获取文件列表

#### 接口URL
> http://{{host}}/list

#### 请求方式
> GET

#### Content-Type
> form-data









## 删除文件

#### 接口URL
> http://{{host}}/file/main-func1-cfg.png

#### 请求方式
> DELETE

#### Content-Type
> form-data









## 删除所有文件

#### 接口URL
> http://{{host}}/files

#### 请求方式
> DELETE

#### Content-Type
> form-data









## 绘图

#### 接口URL
> http://{{host}}/draw?type=callgraph&file=func1.cpp

#### 请求方式
> POST

#### Content-Type
> form-data

#### 请求Query参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| type     | callgraph | 必填 | - |
| file     | func1.cpp | 必填 | - |






#### 成功响应示例
```javascript
{
	"data": {
		"images": [
			"_Z1av-func1-cfg.png",
			"_Z1bv-func1-cfg.png",
			"_Z1cv-func1-cfg.png",
			"_Z1dv-func1-cfg.png",
			"_Z1ev-func1-cfg.png",
			"main-func1-cfg.png"
		]
	},
	"success": true
}
```



## Cmpfun

#### 接口URL
> http://{{host}}/cmpfun?file1=func1.cpp&file2=func2.cpp

#### 请求方式
> GET

#### Content-Type
> form-data

#### 请求Query参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| file1     | func1.cpp | 必填 | - |
| file2     | func2.cpp | 必填 | - |






#### 成功响应示例
```javascript
{
	"data": [
		{
			"func1": "a()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "a()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "b()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "b()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "b()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "b()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "c()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "c()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "c()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "c()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "d()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "e()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "e()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "e()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "e()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "main",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "main",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "main",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "main",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 27,
			"similarity": 1
		}
	],
	"success": true
}
```



## CmpCfg

#### 接口URL
> http://{{host}}/cmpcfg?file1=func1.cpp&file2=func2.cpp

#### 请求方式
> GET

#### Content-Type
> form-data

#### 请求Query参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| file1     | func1.cpp | 必填 | - |
| file2     | func2.cpp | 必填 | - |






#### 成功响应示例
```javascript
{
	"data": [
		{
			"func1": "a()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "a()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "a()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 7,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "b()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "b()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "b()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "b()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 12,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "c()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "c()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "c()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "c()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "c()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 17,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "d()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "d()",
			"func2": "bb()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 12,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "d()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 22,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "e()",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "e()",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "e()",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "e()",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 27,
			"line2": 27,
			"similarity": 1
		},
		{
			"func1": "main",
			"func2": "aa()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 7,
			"similarity": 1
		},
		{
			"func1": "main",
			"func2": "cc()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 17,
			"similarity": 0
		},
		{
			"func1": "main",
			"func2": "dd()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 22,
			"similarity": 0
		},
		{
			"func1": "main",
			"func2": "ee()",
			"file1": "func1.cpp",
			"file2": "func2.cpp",
			"inst1": 1,
			"inst2": 1,
			"line1": 32,
			"line2": 27,
			"similarity": 1
		}
	],
	"success": true
}
```



## 查询进度

#### 接口URL
> http://{{host}}/progress?file1=func1.cpp&file2=func2.cpp

#### 请求方式
> GET

#### Content-Type
> form-data

#### 请求Query参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| file1     | func1.cpp | 必填 | - |
| file2     | func2.cpp | 必填 | - |






#### 成功响应示例
```javascript
{
	"data": 1,
	"success": true
}
```



## Detect

#### 接口URL
> http://{{host}}/detect?file=stack.cpp

#### 请求方式
> GET

#### Content-Type
> form-data

#### 请求Query参数

| 参数        | 示例值   | 是否必填   |  参数描述  |
| :--------   | :-----  | :-----  | :----  |
| file     | stack.cpp | 必填 | - |






#### 成功响应示例
```javascript
{
	"data": [
		{
			"type": "overflow",
			"line": 7,
			"column": 5
		}
	],
	"success": true
}
```


