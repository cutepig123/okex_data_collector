# Feasibility study

## Lib selection options

websocket lib: boost::beast cpprestsdk

package management: vcpkg https://vcpkg.io/en/getting-started.html

```
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
vcpkg install [packages to install]
vcpkg integrate install
```

## learn OKEX 

The Demo Trading URL:
\- REST: `https://www.okex.com`
\- Public WebSocket：`wss://wspap.okex.com:8443/ws/v5/public?brokerId=9999`
\- Private WebSocket：`wss://wspap.okex.com:8443/ws/v5/private?brokerId=9999`

Start API Demo Trading by the following steps:
Login OKEx —> Assets —> Start Demo Trading —> Personal Center —> Demo Trading API -> Create Demo Trading V5 APIKey —> Start your Demo Trading

Note: `x-simulated-trading: 1` needs to be added to the header of the Demo Trading request.



### My API Key:

apikey = "2ed10735-6d2f-4c88-931b-92f3be283000" secretkey = "060F3BB777FEC79D130C3AF014DEA00C" IP = "" 备注名 = "jinshouhe" 权限 = "只读/提现/交易"

### SDK:

https://github.com/okex/V3-Open-API-SDK/tree/master/okex-cpp-sdk-api --> G:\_codes\V3-Open-API-SDK

REST API tested. V3 API is outdated. Need modify to V5

### python Websocket API Test

issue 1: https://github.com/aaugustin/websockets/issues/760

原因分析：这是由于py api的改变不兼容

解决方案：使用py3.8

issue 2： typeError: a bytes-like object is required, not 'str'

原因分析：这是v5 api的改变不兼容

解决方案：删除

issue 3： '{"event":"error","msg":"Illegal request: {\\"op\\": \\"subscribe\\", \\"args\\": [\\"spot/account:USDT\\"]}","code":"60012"}'

the input is '{"op": "subscribe", "args": ["spot/account:USDT"]}'

原因分析：这是v5 api的改变不兼容

解决方案：修改为 channels = [{"channel":"instruments", "instType": "FUTURES"}]

### C++ Websocket API Test

issue 4:  cpp很多地方编译不过

原因分析：On Windows, all strings are wide

解决方案：use following convert function

```
#include <locale> 
#include <codecvt>
std::string convert(std::wstring const& src)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    return converter.to_bytes(src);
}
```

issue5: error C2079: 'ctx' uses undefined struct 'hmac_ctx_st'

原因分析：because it is deprecated

解决方案：change HMAC_CTX_init to HMAC_CTX_new...

issue6:Please add task-based continuation to handle all exceptions coming from tasks.

原因分析：他的机制是会catch task的exception然后在用户wait的时候抛出

解决方案：需要用 try-catch来获取task的结果

## how to get orderbook, balance, and position data 

balance余额: account channel, cashBal

position持仓: position channel, tradeId

思路：订阅订单频道（orders），将订单id（tradeId）等信息记录到一个table

订阅balance_and_position频道，将订单id（tradeId）等信息记录到一个table



测试order

```
connect success: wss://wspap.okex.com:8443/ws/v5/private?brokerId=9999
send success: { "op": "login", "args" : [{"apiKey":"2ed10735-6d2f-4c88-931b-92f3be283000", "passphrase" : "hejinshou123", "timestamp" : "1621701460", "sign" : "sgLpJF/uS5p9BCaU4mPbJiMJzSRzayLCNcI5F6WIpHM="}]}
receive success:
         data: {"event":"login", "msg" : "", "code": "0"}
         datalen: 65536


send success: {"args":[{"channel":"orders","instType":"ANY"}],"op":"subscribe"}
receive success:
         data: {"event":"subscribe","arg":{"channel":"orders","instType":"ANY"}}
         datalen: 65536


send success: {"args":[{"channel":"balance_and_position"}],"op":"subscribe"}
receive success:
         data: {"event":"subscribe","arg":{"channel":"balance_and_position"}}
         datalen: 65536


Press any key to continue . . . receive success:
         data: {"arg":{"channel":"balance_and_position"},"data":[{"balData":[{"cashBal":"0.955","ccy":"BTC","uTime":"1621698321249"},{"cashBal":"10","ccy":"LTC","uTime":"1621605055972"},{"cashBal":"3000","ccy":"TUSD","uTime":"1621605055935"},{"cashBal":"5","ccy":"ETH","uTime":"1621605055956"},{"cashBal":"1000","ccy":"ADA","uTime":"1621605056029"},{"cashBal":"3000","ccy":"USDK","uTime":"1621605055850"},{"cashBal":"4393.0658206","ccy":"USDT","uTime":"1621698321249"},{"cashBal":"500","ccy":"UNI","uTime":"1621605055924"},{"cashBal":"100","ccy":"JFI","uTime":"1621605055902"},{"cashBal":"10000","ccy":"TRX","uTime":"1621605056009"},{"cashBal":"100","ccy":"OKB","uTime":"1621605055873"},{"cashBal":"3000","ccy":"USDC","uTime":"1621605055991"},{"cashBal":"3000","ccy":"PAX","uTime":"1621605055861"}],"eventType":"snapshot","pTime":"1621701461513","posData":[]}]}
         datalen: 65536


 ccy:BTC cashBal:0.955 uTime:1621698321249
 ccy:LTC cashBal:10 uTime:1621605055972
 ccy:TUSD cashBal:3000 uTime:1621605055935
 ccy:ETH cashBal:5 uTime:1621605055956
 ccy:ADA cashBal:1000 uTime:1621605056029
 ccy:USDK cashBal:3000 uTime:1621605055850
 ccy:USDT cashBal:4393.0658206 uTime:1621698321249
 ccy:UNI cashBal:500 uTime:1621605055924
 ccy:JFI cashBal:100 uTime:1621605055902
 ccy:TRX cashBal:10000 uTime:1621605056009
 ccy:OKB cashBal:100 uTime:1621605055873
 ccy:USDC cashBal:3000 uTime:1621605055991
 ccy:PAX cashBal:3000 uTime:1621605055861
receive success:
         data: {"arg":{"channel":"orders","instType":"ANY"},"data":[{"accFillSz":"0","amendResult":"","avgPx":"","cTime":"1621701477747","category":"normal","ccy":"","clOrdId":"","code":"0","execType":"","fee":"0","feeCcy":"USDT","fillFee":"0","fillFeeCcy":"","fillPx":"","fillSz":"0","fillTime":"","instId":"BTC-USDT","instType":"SPOT","lever":"","msg":"","ordId":"316374160234655744","ordType":"market","pnl":"0","posSide":"","px":"","rebate":"0","rebateCcy":"BTC","reqId":"","side":"sell","slOrdPx":"","slTriggerPx":"","state":"live","sz":"0.01","tag":"","tdMode":"cash","tpOrdPx":"","tpTriggerPx":"","tradeId":"","uTime":"1621701477747"}]}
         datalen: 65536


 instType:SPOT instId:BTC-USDT ordId:316374160234655744 tradeId: cTime:1621701477747
receive success:
         data: {"arg":{"channel":"orders","instType":"ANY"},"data":[{"accFillSz":"0.01","amendResult":"","avgPx":"15450.6","cTime":"1621701477747","category":"normal","ccy":"","clOrdId":"","code":"0","execType":"T","fee":"-0.077253","feeCcy":"USDT","fillFee":"-0.077253","fillFeeCcy":"USDT","fillPx":"15450.6","fillSz":"0.01","fillTime":"1621701477752","instId":"BTC-USDT","instType":"SPOT","lever":"","msg":"","ordId":"316374160234655744","ordType":"market","pnl":"0","posSide":"","px":"","rebate":"0","rebateCcy":"BTC","reqId":"","side":"sell","slOrdPx":"","slTriggerPx":"","state":"filled","sz":"0.01","tag":"","tdMode":"cash","tpOrdPx":"","tpTriggerPx":"","tradeId":"69657911","uTime":"1621701477753"}]}
```



## learn cpprest(done)

issue 5: cpp使用websocket编译不过

原因分析：我觉得还是因为vcpkg不够好，不够傻瓜化，当然跟cpp本身傻逼有关

解决方案：https://github.com/microsoft/cpprestsdk/issues/1442

```
.\vcpkg install cpprestsdk[websockets] --recurse
```



https://zhuanlan.zhihu.com/p/75172306

https://github.com/will-henderson/Okex-Cpp

https://blog.csdn.net/qq_25863231/article/details/100174513

## learn MongoDB (done)

.\vcpkg install mongo-cxx-driver

install server https://www.mongodb.com/try/download/community



| SQL术语/概念 | MongoDB术语/概念 | 解释/说明                           |
| :----------- | :--------------- | :---------------------------------- |
| database     | database         | 数据库                              |
| table        | collection       | 数据库表/集合                       |
| row          | document         | 数据记录行/文档                     |
| column       | field            | 数据字段/域                         |
| index        | index            | 索引                                |
| table joins  |                  | 表连接,MongoDB不支持                |
| primary key  | primary key      | 主键,MongoDB自动将_id字段设置为主键 |

https://www.runoob.com/mongodb/mongodb-window-install.html

```
C:\mongodb\bin\mongod --dbpath c:\data\db
mongo
db.runoob.insert({x:10})
db.runoob.find()
show dbs
db.runoob.insert({"name":"菜鸟教程"})
use runoob
db
show tables
```





## learn redis (done)

redis-plus-plus

https://www.runoob.com/redis/redis-install.html

```
redis-server.exe
redis-cli.exe
set myKey abc
get myKey
```

redis有几种用法

- 作为k-v存储系统：需要定义唯一的key，对于我们的系统
  - key： 前缀 + 自增key？，比如order1, order2; balance1, balance2
- 作为发布订阅系统

## unit test (done)

boost test

googletest .\vcpkg install gtest

issue1: https://stackoverflow.com/questions/46025439/unresolved-external-symbol-error-with-google-mock-and-vcpkg

原因分析：我觉得还是vcpkg弄得不够好

解决方案：Add "GTEST_LINKED_AS_SHARED_LIBRARY" preprocessor definiton to your project. See : github.com/google/googletest/issues/292