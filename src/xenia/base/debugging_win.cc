/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/debugging.h"

#include <sstream>
#include <string>

#include "xenia/base/platform_win.h"
#include "xenia/base/string_buffer.h"

namespace xe {
namespace debugging {

bool IsDebuggerAttached() { return IsDebuggerPresent() ? true : false; }

void Break() { __debugbreak(); }

// Dialog procedure for the custom assertion dialog
INT_PTR CALLBACK AssertionDialogProc(HWND hwndDlg, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
  switch (message) {
    case WM_INITDIALOG:
      // Center the dialog on screen
      {
        RECT rc;
        GetWindowRect(hwndDlg, &rc);
        int xPos = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
        int yPos = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
        SetWindowPos(hwndDlg, NULL, xPos, yPos, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER);
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDABORT:
          EndDialog(hwndDlg, 0);  // Abort
          return TRUE;
        case IDRETRY:
          EndDialog(hwndDlg, 1);  // Retry (break into debugger)
          return TRUE;
        case IDIGNORE:
          EndDialog(hwndDlg, 2);  // Ignore
          return TRUE;
        case IDCANCEL:
          EndDialog(hwndDlg, 2);  // Treat Cancel as Ignore
          return TRUE;
      }
      break;
  }
  return FALSE;
}

int ShowAssertionDialog(const char* message, const char* file, int line) {
  // Build the full error message
  std::ostringstream oss;
  oss << "Assertion Failed!\n\n";
  oss << "File: " << file << "\n";
  oss << "Line: " << line << "\n\n";
  oss << "Expression: " << message << "\n\n";
  oss << "Select an action:\n";
  oss << "  Abort  - Terminate the application\n";
  oss << "  Retry  - Break into the debugger\n";
  oss << "  Ignore - Continue execution (may cause instability)";

  std::string error_text = oss.str();

  // Create a task dialog (modern Windows dialog with better UI)
  TASKDIALOGCONFIG config = {0};
  config.cbSize = sizeof(config);
  config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT |
                   TDF_ENABLE_HYPERLINKS;
  config.dwCommonButtons = 0;  // Custom buttons

  // Define custom buttons
  TASKDIALOG_BUTTON buttons[] = {
      {IDABORT, L"Abort\nTerminate the application"},
      {IDRETRY, L"Retry\nBreak into debugger"},
      {IDIGNORE, L"Ignore\nContinue execution"}};

  config.pButtons = buttons;
  config.cButtons = 3;
  config.nDefaultButton = IDRETRY;

  // Convert strings to wide char
  std::wstring wtitle = L"Xenia-XT - Assertion Failed";
  std::wstring wmain = L"A runtime assertion has failed";

  // For the content, we'll use expandable information to show the copyable
  // text
  std::wstring wcontent(error_text.begin(), error_text.end());

  config.pszWindowTitle = wtitle.c_str();
  config.pszMainInstruction = wmain.c_str();
  config.pszContent = nullptr;
  config.pszExpandedInformation = wcontent.c_str();
  config.pszCollapsedControlText = L"Show details";
  config.pszExpandedControlText = L"Hide details";
  config.dwFlags |= TDF_EXPANDED_BY_DEFAULT;

  config.pszMainIcon = TD_ERROR_ICON;

  int button_pressed = 0;
  HRESULT hr = TaskDialogIndirect(&config, &button_pressed, NULL, NULL);

  if (FAILED(hr)) {
    // Fallback to MessageBox if TaskDialog fails
    int result = MessageBoxA(
        NULL, error_text.c_str(), "Xenia - Assertion Failed",
        MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);

    switch (result) {
      case IDABORT:
        return 0;
      case IDRETRY:
        return 1;
      case IDIGNORE:
        return 2;
      default:
        return 0;
    }
  }

  // Map button press to result
  switch (button_pressed) {
    case IDABORT:
      return 0;  // Abort
    case IDRETRY:
      return 1;  // Retry (break)
    case IDIGNORE:
      return 2;  // Ignore
    default:
      return 0;  // Default to abort
  }
}

namespace internal {
void DebugPrint(const char* s) { OutputDebugStringA(s); }
}  // namespace internal

}  // namespace debugging
}  // namespace xe
