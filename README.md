# ESP32 附近WiFi设备嗅探

## 介绍

本项目是基于ESP32的WiFi嗅探器，可以扫描附近的WiFi使用设备

## 技术细节

Wi-Fi的嗅探是通过ESP32提供的WiFi数据包嗅探功能实现的，具体的实现过程如下：

1. 通过`esp_wifi_set_promiscuous`函数设置ESP32为嗅探模式
2. 通过`esp_wifi_set_promiscuous_rx_cb`函数设置嗅探回调函数
3. 在回调函数中解析WiFi数据包, 获取MAC地址:
    - WiFi的数据包有两种类型，一种是管理帧，一种是数据帧
    - 管理帧中的MAC地址在`wifi_promiscuous_pkt_t`结构体中的`payload`字段中
    - 根据IEEE802.11标准, 管理包的结构如下:
        ![Management Packet](https://user-images.githubusercontent.com/32300164/202865799-91dc5536-18bb-419a-ba38-78444a9c3585.png)
        其中Frame Control的结构如下:
        ![Frame Control](https://user-images.githubusercontent.com/32300164/202865866-8510bc74-8b14-404c-a0ed-8cb602731056.png)
        为了获得正确的MAC地址, 我们需要关心From DS和To DS这两个字段, 其含义如下:
        ![From DS and To DS](https://user-images.githubusercontent.com/32300164/202865990-554a9f0b-527d-4f2e-8bcc-bb5e62997f41.png)
    - 从结构体中读取需要的数据
4. 将MAC地址转换为字符串，通过屏幕打印出来

## 展示

<!--markdownlint-disable MD033-->
<details>

![image](https://user-images.githubusercontent.com/32300164/202866285-82a3279e-e3e2-4b19-807f-fab645667c09.png)

</details>
