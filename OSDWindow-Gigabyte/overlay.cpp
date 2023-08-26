//CREDITS -> https://github.com/iraizo/nvidia-overlay-hijack for overlay

#include "overlay.hpp"
#include <comdef.h>
#include <corecrt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool IsRunningAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

bool Overlay::init()
{
    keybd_event(VK_CAPITAL, 0, KEYEVENTF_EXTENDEDKEY, 0);
    Sleep(100);
    keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);

    Sleep(3000); //Wait a bit for the thread to appear

    if (IsRunningAsAdmin())
    {
        DWORD threadId = 0; // The target thread ID
        printf("Enter Thread PID show in POC video -> ");     //TODO: Make it automatically get the thread ID
        scanf("%i", &threadId);
        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadId);

        if (hThread)
            SuspendThread(hThread);
    }

   
    window = FindWindowA("WindowsForms10.Window.0.app.0.141b42a_r6_ad1", "FloatingNativeWindow");

    if (!window)
    {
        printf("Fail!\n");
        return false;
    }

    SetWindowLongA(window, -20, static_cast<LONG_PTR>((static_cast<int>(GetWindowLongA(window, -20)) | 32)));
    MoveWindow(window, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN), false);
    MARGINS margin = { -2, -2, -2, -2 };
    DwmExtendFrameIntoClientArea(window, &margin);
    if (!SetLayeredWindowAttributes(window, 0, 255, 2))
    {
        printf("Fail!\n");
        return false;
    }

    SetWindowDisplayAffinity(window, WDA_EXCLUDEFROMCAPTURE);


    return true;
    
}

bool Overlay::startup_d2d()
{
    RECT rc;
    if (HRESULT ret = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory) == S_OK)
    {
        if (ret = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)(&write_factory)) == S_OK)
        {
            /* not adding more checks here, this should all succeed. Should be pretty safe. */
            write_factory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"en-us", &format);
            GetClientRect(window, &rc);

            if (ret = d2d_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &tar) == S_OK)
            {
                D2D1_BRUSH_PROPERTIES properties = { 1.0f, D2D1::Matrix3x2F::Identity() };

                tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), properties, &brush);

                return true;
            }
        }
    }
    return false;
}

void Overlay::begin_scene()
{
    tar->BeginDraw();
}

void Overlay::end_scene()
{
    tar->EndDraw();
}

void Overlay::clear_scene()
{
    tar->Clear();
}

void Overlay::draw_text(int x, int y, const char* text, D2D1::ColorF color, ...)
{
    char    buffer[4096];
    int     length;
    wchar_t out[256];
    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return;

    va_list arg_list;
    va_start(arg_list, text);
    vsnprintf(buffer, sizeof(buffer), text, arg_list);
    va_end(arg_list);

    length = strlen(buffer);

    mbstowcs(out, buffer, length);

    tar->DrawTextA(out, length, format, D2D1::RectF(x, y, window_metrics.right - window_metrics.left, window_metrics.bottom - window_metrics.top), brush);
    brush->SetColor(color);
}


