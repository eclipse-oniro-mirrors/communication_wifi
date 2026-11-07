# ohos-wifiManager

## 概述

WiFi 控制工具，提供 STA 模式控制、WiFi 扫描以及个人热点（AP）管理能力。

## 功能列表

- **sta-enable**: 启用 WiFi STA 模式
- **sta-disable**: 禁用 WiFi STA 模式
- **scan-start**: 启动 WiFi 扫描
- **scan-list**: 列出扫描结果
- **sta-connect**: 连接WiFi
- **sta-getLinkedInfo**: 查看网络连接状态
- **hotspot-enable** / **hotspot-disable**：打开/关闭热点
- **hotspot-config-set**：配置热点 SSID/密码/频段
- **hotspot-config-get**：查询当前配置与运行状态
- **hotspot-blocklist-get**：查询黑名单列表（增/删请在系统设置中操作）

## 依赖

- 系统能力：`SystemCapability.Communication.WiFi.STA`、`SystemCapability.Communication.WiFi.AP`
- 权限：
  - `ohos.permission.cli.MANAGE_WIFI_TOGGLE`
  - `ohos.permission.cli.MANAGE_WIFI_SCAN`
  - `ohos.permission.cli.MANAGE_WIFI_CONNECT`
  - `ohos.permission.MANAGE_WIFI_HOTSPOT`
  - `ohos.permission.GET_WIFI_INFO`
  - `ohos.permission.SET_WIFI_INFO`
  - `ohos.permission.SET_WIFI_CONFIG`
  - `ohos.permission.GET_WIFI_CONFIG`

## 安装路径

```
/system/bin/cli_tool/executable/ohos-wifiManager
```