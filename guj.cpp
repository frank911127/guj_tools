#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    char title[256];
    GetWindowTextA(hWnd, title, sizeof(title));

    if (IsWindowVisible(hWnd) && strlen(title) > 0)
    {
        std::string windowTitle(title);

        if (windowTitle.find("嘉膛") != std::string::npos)
        {
            HWND* pTargetHwnd = (HWND*)lParam;
            *pTargetHwnd = hWnd;
            return FALSE; // 找到就停止列舉
        }
    }
    return TRUE;
}

int main()
{
    // 1. 啟動 GuJian.exe
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL result = CreateProcessA(
        "GuJian.exe",  // 執行的檔案
        NULL,          // 命令列參數
        NULL, NULL, FALSE,
        0,             // 正常啟動
        NULL, NULL, &si, &pi
    );

    if (!result)
    {
        std::cout << "❌ 無法啟動 GuJian.exe，請確認路徑正確！" << std::endl;
        return 1;
    }

    std::cout << "✔ GuJian.exe 啟動中..." << std::endl;

    // 2. 自動重試等待模式
    HWND targetHwnd = NULL;
    int retryCount = 0;
    const int maxRetries = 120; // 最多等兩分鐘（120次，每次0.5秒）

    while (retryCount < maxRetries)
    {
        EnumWindows(EnumWindowsProc, (LPARAM)&targetHwnd);
        if (targetHwnd != NULL)
        {
            std::cout << "✔ 找到目標視窗!" << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 每0.5秒找一次
        retryCount++;
    }

    if (targetHwnd == NULL)
    {
        std::cout << "❌ 超時找不到目標視窗，結束程式！" << std::endl;
        return 1;
    }

    // 3. 修改視窗樣式
    int style = (int)GetWindowLongPtr(targetHwnd, GWL_STYLE);
    if (!style)
    {
        std::cout << "讀取樣式失敗!" << std::endl;
        return 1;
    }

    style &= ~WS_CAPTION; // 移除標題列

    auto rtn = SetWindowLongPtr(targetHwnd, GWL_STYLE, style);
    if (rtn == NULL)
    {
        std::cout << "寫入樣式失敗!" << std::endl;
        return 1;
    }

    ShowWindow(targetHwnd, SW_MAXIMIZE); // 最大化顯示

    std::cout << "✔ 視窗樣式修改完成，程式即將自動結束！" << std::endl;

    return 0; // 自動結束
}
