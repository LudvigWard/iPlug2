 /*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#pragma once

#include "IPlugPlatform.h"
#include "wdlstring.h"
#include <functional>

#if defined OS_MAC
  #define PLATFORM_VIEW NSView
  #define PLATFORM_RECT NSRect
  #define MAKERECT NSMakeRect
#elif defined OS_IOS
  #define PLATFORM_VIEW UIView
  #define PLATFORM_RECT CGRect
  #define MAKERECT CGRectMake
#elif defined OS_WIN
  #include <wrl.h>
  #include <wil/com.h>
  #include "WebView2.h"
  #include "WebView2EnvironmentOptions.h"
#endif

BEGIN_IPLUG_NAMESPACE

using completionHandlerFunc = std::function<void(const char* result)>;

/** IWebView is a base interface for hosting a platform web view inside an IPlug plug-in's UI */
class IWebView
{
public:
  IWebView(bool opaque = true);
  virtual ~IWebView();
  
  void* OpenWebView(void* pParent, float x, float y, float w, float h, float scale = 1.0f, bool enableDevTools = true);
  void CloseWebView();
  void HideWebView(bool hide);
  
  /** Load an HTML string into the webview */
  void LoadHTML(const char* html);
  
  /** Instruct the webview to load an external URL */
  void LoadURL(const char* url);
  
  /** Load a file on disk into the web view
   * @param fileName On windows this should be an absolute path to the file you want to load. 
   * On macOS/iOS it can just be the file name if the file is packaged into a subfolder "web" of the bundle resources
   * @param bundleID The NSBundleID of the macOS/iOS bundle, not required on Windows
   * @param useCustomScheme If true, uses a custom url scheme
   * This means that the webview content is served as if it was on a web server (required for some web frameworks */
  void LoadFile(const char* fileName, const char* bundleID = "", bool useCustomScheme = false);
  
  /** Trigger a reload of the webview's content */
  void ReloadPageContent();
  
  /** Runs some JavaScript in the webview
   * @param scriptStr UTF8 encoded JavaScript code to run
   * @param func A function conforming to completionHandlerFunc that should be called on successful execution of the script */
  void EvaluateJavaScript(const char* scriptStr, completionHandlerFunc func = nullptr);
  
  /** Enable scrolling on the webview. NOTE: currently only implemented for iOS */
  void EnableScroll(bool enable);
  
  /** Sets whether the webview is interactive */
  void EnableInteraction(bool enable);
  
  /** Set the bounds of the webview in the parent window. xywh are specifed in relation to a 1:1 non retina screen */
  void SetWebViewBounds(float x, float y, float w, float h, float scale = 1.);

  /** Called when the web view is ready to receive navigation instructions */
  virtual void OnWebViewReady() {}
  
  /** Called after navigation instructions have been exectued and e.g. a page has loaded */
  virtual void OnWebContentLoaded() {}
  
  /** When a script in the web view posts a message, it will arrive as a UTF8 json string here */
  virtual void OnMessageFromWebView(const char* json) {}
  
  /** Override to filter URLs */
  virtual bool CanNavigateToURL(const char* url) { return true; }
  
  /** Override to filter Mime types that should be downloaded */
  virtual bool CanDownloadMIMEType(const char* mimeType) { return true; }
  
  /** Override to download the file to a specific location other than e.g. NSTemporaryDirectory */
  virtual void GetLocalDownloadPathForFile(const char* fileName, WDL_String& localPath);

  /** Override to handle file download success */
  virtual void DidDownloadFile(const char* path) {};
  
  /** Override to handle file download failure */
  virtual void FailedToDownloadFile(const char* path) {};

  /** Override to handle file download progress */
  virtual void DidReceiveBytes(size_t numBytesReceived, size_t totalNumBytes) {};

  /** Fills the path where web content is being served from, when LoadFile() is used */
  void GetWebRoot(WDL_String& path) const { path.Set(mWebRoot.Get()); }
  
private:
  WDL_String mWebRoot;
  bool mOpaque = true;
#if defined OS_MAC || defined OS_IOS
  void* mWKWebView = nullptr;
  void* mWebConfig = nullptr;
  void* mScriptHandler = nullptr;
#elif defined OS_WIN
  HWND mParentWnd = NULL;
  wil::com_ptr<ICoreWebView2Controller> mWebViewCtrlr;
  wil::com_ptr<ICoreWebView2> mCoreWebView;
  wil::com_ptr<ICoreWebView2Environment> mWebViewEnvironment;
  EventRegistrationToken mWebMessageReceivedToken;
  EventRegistrationToken mNavigationCompletedToken;
  EventRegistrationToken mContextMenuRequestedToken;
  EventRegistrationToken mDownloadStartingToken;
  EventRegistrationToken mBytesReceivedChangedToken;
  EventRegistrationToken mStateChangedToken;
  bool mShowOnLoad = true;
#endif
};

END_IPLUG_NAMESPACE
