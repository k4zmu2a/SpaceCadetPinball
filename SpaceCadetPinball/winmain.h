#pragma once

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
static void winmain_memalloc_failure();
static LRESULT CALLBACK message_handler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
int ProcessWindowMessages();
