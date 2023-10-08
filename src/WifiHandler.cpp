#include "WifiHandler.h"

const char *wifi_tag = "WifiHandler";

#define CONNECTION_TIMEOUT 1

WifiHandler::WifiHandler(const char *ssid, const char *pass) : _ssid(ssid), _pass(pass)
{
}

WifiHandler::~WifiHandler()
{
}

void WifiHandler::checkStatus()
{
    switch (WiFi.status())
    {
    case WL_NO_SSID_AVAIL:
        ESP_LOGI(wifi_tag, "WiFi SSID not found");
        break;
    case WL_CONNECT_FAILED:
        ESP_LOGE(wifi_tag, "WiFi failed - WiFi not connected! Reason: ");
        return;
        break;
    case WL_CONNECTION_LOST:
        ESP_LOGI(wifi_tag, "WiFi connection was lost");
        break;
    case WL_SCAN_COMPLETED:
        ESP_LOGI(wifi_tag, "WiFi scan is completed");
        break;
    case WL_DISCONNECTED:
        ESP_LOGW(wifi_tag, "WiFi is disconnected");
        break;
    case WL_CONNECTED:
        ESP_LOGI(wifi_tag, "Wifi connected to SSID : %s | strength : %d", WiFi.SSID().c_str(), WiFi.RSSI());
        return;
        break;
    default:
        ESP_LOGI(wifi_tag, "WiFi Status: %d", WiFi.status());
        break;
    }
}

void WifiHandler::init()
{
    ESP_LOGI(wifi_tag, "Connecting to WiFi...");
    WiFi.begin(_ssid, _pass);

    // Wait for the WiFi to connect or reach the connection timeout
    auto tryDelay = 500;
    auto numberOfTries = 0;

    while (WiFi.status() != WL_CONNECTED && numberOfTries <= CONNECTION_TIMEOUT)
    {
        WifiHandler::checkStatus();
        vTaskDelay(tryDelay);
        numberOfTries++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        ESP_LOGW(wifi_tag, "Failed to connect to WiFi! | num of tries : %d", numberOfTries);
        // Continue with task creation even if the connection failed
    }
    else
    {
        WifiHandler::checkStatus();
    }

    // Create the task regardless of the connection status
    xTaskCreate(&WifiHandler::_staticTaskFunc,
                "wifi task handler",
                4096,
                this,
                1,
                &_taskHandle);
}

/*STATIC*/ void WifiHandler::_staticTaskFunc(void *pvParam)
{
    WifiHandler *wifiHandlerObj =
        reinterpret_cast<WifiHandler *>(pvParam);

    wifiHandlerObj->_taskFunc();
}

void WifiHandler::_taskFunc()
{
    ESP_LOGI(wifi_tag, "Wifi task started !");
    int reconnectAttempts = 0;
    const int maxReconnectAttempts = 3;
    const TickType_t reconnectInterval = pdMS_TO_TICKS(60000); // 1 minutes in ticks

    TickType_t lastReconnectTime = xTaskGetTickCount();

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            ESP_LOGW(wifi_tag, "WiFi disconnected, trying to reconnect...");
            WiFi.disconnect();
            WiFi.reconnect();

            // Wait for the connection to be established or timeout
            unsigned long startMillis = xTaskGetTickCount();
            while (WiFi.status() != WL_CONNECTED && xTaskGetTickCount() - startMillis < pdMS_TO_TICKS(5000))
            {
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                WifiHandler::checkStatus();
                reconnectAttempts = 0; // Reset the reconnect attempts counter
            }
            else
            {
                ESP_LOGE(wifi_tag, "Failed to reconnect to WiFi!");
                reconnectAttempts++;

                if (reconnectAttempts >= maxReconnectAttempts)
                {
                    WiFi.disconnect();
                    ESP_LOGI(wifi_tag, "Waiting for the next reconnection attempt...");
                    vTaskDelayUntil(&lastReconnectTime, reconnectInterval);
                    reconnectAttempts = 0; // Reset the reconnect attempts counter
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}
