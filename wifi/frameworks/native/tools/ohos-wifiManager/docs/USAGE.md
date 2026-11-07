# 使用说明

## 基本用法

```bash
ohos-wifiManager <command>
```

## 命令列表

| 命令 | 说明 | 权限 | 前置依赖 |
|------|------|------|----------|
| sta-enable | 启用 WiFi STA 模式 | ohos.permission.cli.MANAGE_WIFI_TOGGLE | 无 |
| sta-disable | 禁用 WiFi STA 模式 | ohos.permission.cli.MANAGE_WIFI_TOGGLE | 无 |
| scan-start | 启动 WiFi 扫描 | ohos.permission.cli.MANAGE_WIFI_SCAN | 无 |
| scan-list | 列出扫描结果 | ohos.permission.cli.MANAGE_WIFI_SCAN | 无 |
| sta-connect | 连接WiFi | ohos.permission.cli.MANAGE_WIFI_SCAN + ohos.permission.cli.MANAGE_WIFI_CONNECT | 无 |
| hotspot-enable | 打开热点 | ohos.permission.MANAGE_WIFI_HOTSPOT + ohos.permission.GET_WIFI_INFO | 无 |
| hotspot-disable | 关闭热点 | ohos.permission.MANAGE_WIFI_HOTSPOT + ohos.permission.GET_WIFI_INFO | 无 |
| hotspot-config-set | 配置热点 SSID/密码/频段 | ohos.permission.SET_WIFI_INFO + ohos.permission.GET_WIFI_INFO + ohos.permission.SET_WIFI_CONFIG + ohos.permission.GET_WIFI_CONFIG | 无 |
| hotspot-config-get | 查询热点配置与运行状态 | ohos.permission.GET_WIFI_INFO + ohos.permission.GET_WIFI_CONFIG | 无 |
| hotspot-blocklist-get | 查询黑名单列表 | ohos.permission.MANAGE_WIFI_HOTSPOT + ohos.permission.GET_WIFI_INFO | 无 |

## 示例

### 启用 WiFi STA 模式

```bash
ohos-wifiManager sta-enable
```

### 禁用 WiFi STA 模式

```bash
ohos-wifiManager sta-disable
```

### 启动 WiFi 扫描

```bash
ohos-wifiManager scan-start
```

### 列出扫描结果

```bash
ohos-wifiManager scan-list
```

### 连接WiFi

```bash
ohos-wifiManager sta-connect --ssid <ssid> [--preSharedKey <preSharedKey>]
```

### 查看网络连接状态
```bash
ohos-wifiManager sta-getLinkedInfo
```

### 打开 / 关闭热点
```bash
ohos-wifiManager hotspot-enable
ohos-wifiManager hotspot-disable
```
- 幂等调用：热点本就处于目标状态时同样返回成功

### 配置热点（SSID/密码/频段）
```bash
ohos-wifiManager hotspot-config-set --ssid MyHotspot --preSharedKey 12345678 --band 5G
```
- `--preSharedKey`：热点密码，8~63 位
- `--band` 取值：`2.4G` / `5G`（若设备不支持所选频段将报错；可用 `hotspot-config-get` 的 `validBands` 查询设备实际支持的频段）
- 仅需修改的部分可省略，未指定项保持当前配置
- 热点开启中，配置将实时下发并立即生效；热点未开启时，配置将于下次开启热点时生效

### 查询热点配置与运行状态
```bash
ohos-wifiManager hotspot-config-get
```

输出包含 `validBands`（设备支持的热点频段列表）；密码不回显明文，以 `hasPassword` 布尔值表示。

### 黑名单查询
```bash
ohos-wifiManager hotspot-blocklist-get
```
- 返回已拉黑的设备：`data.stations[]`（`bssid` / `ipAddr` / `deviceName`）+ `data.count`
- `bssid` 与 `ipAddr` 恒为脱敏值；`deviceName` 缺失时为空串；热点未开启时也可查询（黑名单为持久化数据）

## 输出格式

进程退出码：`0` = 成功，`1` = 失败，与 JSON `status` 一致；无参数或未知命令时输出帮助列表并返回 `1`。

命令成功时返回 JSON 格式结果：

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "message": "WiFi STA mode enabled successfully"
  }
}
```

或扫描结果列表：

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "networks": [
      {
        "ssid": "MyWiFi",
        "bssid": "12:34:56:**:**:**",
        "securityType": 5,
        "rssi": -45,
        "frequency": 2437
      }
    ],
    "count": 1
  }
}
```

命令失败时返回错误信息（`suggestion` 为修复建议，参数错误时附 Usage 引导）：

```json
{
  "type": "result",
  "status": "failed",
  "errCode": "WIFI_ERROR",
  "errMsg": "WiFi cannot be enabled in airplane mode",
  "suggestion": "Check WiFi permissions and airplane mode"
}
```