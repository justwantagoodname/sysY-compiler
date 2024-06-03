# SysY Compiler
Hope I can finish it...

# Usage
## 在 `Ubuntu` 与 `Debian` 发行版下安装编译环境
没有 `make`, 先安装 `make`
```shell
$ make requirements
```

## 启动 HotReload 
此时会自动监听 `src` 目录下的文件变化, 并自动编译
*不支持新建文件, 需要手动重启*
```shell
$ make dev
```

## 仅编译
```shell
$ make
```

## 清理目录
```shell
$ make clean
```