# 配置模块

`src/config` 负责应用配置文件路径和应用级配置读写。配置文件使用 TOML 格式，由 toml++ 解析，不使用正则表达式解析。

## 配置文件位置

`AppPaths` 将用户可写数据放在 Qt 的 `QStandardPaths::GenericDataLocation` 下的 `OJ-client` 目录中。具体路径由运行环境决定，Windows 通常位于用户的 AppData 目录。

| 接口 | 返回路径 |
| --- | --- |
| `AppPaths::baseDir()` | 用户数据根目录 `<GenericDataLocation>/OJ-client`。调用时会创建目录。 |
| `AppPaths::dataDir()` | `baseDir()/data`，调用时会创建目录。 |
| `AppPaths::logDir()` | `baseDir()/log.log`。 |
| `AppPaths::cacheDir()` | `baseDir()/cache`，调用时会创建目录。 |
| `AppPaths::AIconfigPath()` | `baseDir()/AIconfig.toml`。 |
| `AppPaths::appconfigPath()` | `baseDir()/appconfig.toml`。 |

仓库中的 `config/appconfig.toml` 是格式样例，不是程序运行时默认读取的位置。

## `appconfig.toml` 格式

当前应用配置包含两个表：

```toml
[network]
ServerUrl = "http://10.129.240.62:18080"

[alarm]
enabled = false
ring_path = 'E:\OJ-client\vedio\hachimi.mp3'
```

| TOML 字段 | 类型 | 含义 |
| --- | --- | --- |
| `network.ServerUrl` | 字符串 | 判题服务基础 URL。 |
| `alarm.enabled` | 布尔值 | 是否启用截止时间提醒。 |
| `alarm.ring_path` | 字符串 | 提醒播放的音频文件路径。 |

路径、URL 和其他字符串必须使用合法的 TOML 字符串写法。Windows 路径可使用字面量字符串（单引号），这样反斜杠不需要额外转义。

## `AppConfig` 接口

所有读取函数都是同步函数，读取 `AppPaths::appconfigPath()`。文件不存在、无法读取、TOML 语法错误或字段类型不匹配时，读取函数返回默认值。

| 接口 | 返回值和行为 |
| --- | --- |
| `loadAlarmEnabled(bool defaultValue = false)` | 返回 `alarm.enabled`；字段不存在或不是布尔值时返回 `defaultValue`。 |
| `loadRingPath()` | 返回 `alarm.ring_path`；读取失败或字段不存在时返回空字符串。 |
| `loadServerUrl()` | 返回 `network.ServerUrl`；读取失败或字段不存在时返回空字符串。 |
| `saveAlarmEnabled(bool enabled, QString *errorMessage = nullptr)` | 更新 `alarm.enabled`，成功返回 `true`，失败返回 `false`。 |
| `saveServerUrl(const QString &value, QString *errorMessage = nullptr)` | 更新 `network.ServerUrl`，成功返回 `true`，失败返回 `false`。 |
| `saveRingPath(const QString &path, QString *errorMessage = nullptr)` | 更新 `alarm.ring_path`，成功返回 `true`，失败返回 `false`。 |

保存操作会保留其他已解析的 TOML 字段，并通过临时文件提交最终文件。成功保存后，`errorMessage` 不需要读取；失败时如果传入非空指针，会写入文件读取、TOML 解析、目录创建或文件写入错误说明。

示例：

```cpp
QString errorMessage;
if (!AppConfig::saveServerUrl(url, &errorMessage)) {
    qWarning() << errorMessage;
}

const bool alarmEnabled = AppConfig::loadAlarmEnabled(false);
const QString ringPath = AppConfig::loadRingPath();
```
