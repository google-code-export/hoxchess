; -------------------------------------------------------------------------
; Purpose: The Windows Installer for HOXChess.
; Author:  Huy Phan
; Update:  Mar 28, 2009
;
; Reference: http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html
; -------------------------------------------------------------------------

;--------------------------------
; Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
; Build environment

  !define PRODUCT_NAME          "HOXChess"
  !define PRODUCT_VERSION       "0.7.0.0"
  !define PRODUCT_PUBLISHER     "PlayXiangqi.com"
  !define PRODUCT_WEB_SITE      "http://www.playxiangqi.com"
  !define PRODUCT_CONFIG_REGKEY "Software\HOXChess"
  !define PRODUCT_DIR_REGKEY    "Software\Microsoft\Windows\CurrentVersion\App Paths\hox_Client.exe"
  !define PRODUCT_UNINST_KEY    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

;--------------------------------
; General

  Name              "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  OutFile           "${PRODUCT_NAME}-${PRODUCT_VERSION}-Setup.exe"
  InstallDir        "$PROGRAMFILES\${PRODUCT_NAME}"
  InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""
  ShowInstDetails   show
  ShowUnInstDetails show

  ; Request admin privileges for Windows Vista
  ; ... to get around this problem:
  ;  http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
  RequestExecutionLevel admin

  SetCompressor /SOLID LZMA

;--------------------------------
; Variables

  Var PRODUCT_LANGUAGE

;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING
  ;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
  ;!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

;--------------------------------
; Language Selection Dialog Settings

  ; Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
  !define MUI_LANGDLL_REGISTRY_KEY ${PRODUCT_CONFIG_REGKEY}
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
; Pages

  ;!insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  ; NOTE: NOT RUNNING directly since it needs to be run within a specific folder.
  ;       !define MUI_FINISHPAGE_RUN "$INSTDIR\bin\hox_Client.exe"
  !define MUI_FINISHPAGE_RUN
  !define MUI_FINISHPAGE_RUN_NOTCHECKED
  !define MUI_FINISHPAGE_RUN_TEXT "Launch ${PRODUCT_NAME}"
  !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
  !insertmacro MUI_PAGE_FINISH

  ;Uninstaller
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

  !define MUI_LANGDLL_ALLLANGUAGES          ; Do not hide any unsupported languages
  !insertmacro MUI_LANGUAGE "English"       ; The 1st is the default language
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "Vietnamese"
  
;--------------------------------
; Descriptions

  ; Language strings
  
  LangString S_SecMain ${LANG_ENGLISH} "Required program files, images, and piece sets."
  LangString S_SecMain ${LANG_SIMPCHINESE} "所需的程序文件，图片和作品集。"
  LangString S_SecMain ${LANG_VIETNAMESE} "Chương trình và mình ảnh cần thiết để chạy."
  
  LangString S_Found_Old ${LANG_ENGLISH} "An old version of ${PRODUCT_NAME} was found in $\n $INSTDIR $\n $\n Click 'OK' to remove it or 'Cancel' to abort this upgrade."
  LangString S_Found_Old ${LANG_SIMPCHINESE} "An old version of ${PRODUCT_NAME} was found in $\n $INSTDIR $\n $\n Click 'OK' to remove it or 'Cancel' to abort this upgrade."
  LangString S_Found_Old ${LANG_VIETNAMESE} "Một chương trình cũ của ${PRODUCT_NAME} tìm ỏ đây $\n $INSTDIR $\n $\n Click 'OK' to remove it or 'Cancel' to abort this upgrade."


;--------------------------------
; Reserve Files

  ; If you are using solid compression, files that are required before
  ; the actual installation should be stored first in the data block,
  ; because this will make your installer start faster.

  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
; Installer Sections

Section "HOXChess" SecMain
  SetOutPath "$INSTDIR\bin"
  File "bin\hox_Client.exe"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\hox_Client.exe"
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\hox_Client.exe"

  SetOutPath "$INSTDIR\resource\locale\zh_CN"
  File "resource\locale\zh_CN\*.*"
  SetOutPath "$INSTDIR\resource\locale\vi_VN"
  File "resource\locale\vi_VN\*.*"

  SetOutPath "$INSTDIR\resource\pieces\1"
  File "resource\pieces\1\*.*"
  SetOutPath "$INSTDIR\resource\pieces\chessvariants_42x42"
  File "resource\pieces\chessvariants_42x42\*.*"
  SetOutPath "$INSTDIR\resource\pieces\wikipedia_60x60"
  File "resource\pieces\wikipedia_60x60\*.*"
  SetOutPath "$INSTDIR\resource\pieces\xqwizard_1_57x57"
  File "resource\pieces\xqwizard_1_57x57\*.*"
  SetOutPath "$INSTDIR\resource\pieces\xqwizard_2_57x57"
  File "resource\pieces\xqwizard_2_57x57\*.*"
  SetOutPath "$INSTDIR\resource\pieces\iXiangQi_55x55"
  File "resource\pieces\iXiangQi_55x55\*.*"

  SetOutPath "$INSTDIR\resource\images"
  File "resource\images\*.png"
  SetOutPath "$INSTDIR\resource\sounds"
  File "resource\sounds\*.wav"
  SetOutPath "$INSTDIR\plugins"
  File "plugins\*.*"

  SetOutPath "$INSTDIR"
  File "README.txt"
  
  ; Force to delete the old user-configuration.
  DeleteRegKey HKCU "${PRODUCT_CONFIG_REGKEY}"
  
  ; Set the default language.
  WriteRegStr HKCU "${PRODUCT_CONFIG_REGKEY}\Options" "language" $PRODUCT_LANGUAGE

  ; Set XQWLight as the default AI.
  WriteRegStr HKCU "${PRODUCT_CONFIG_REGKEY}\Options" "defaultAI" "AI_XQWLight"
SectionEnd

; Microsoft Visual C++ 2008 Redistributable Package (x86)
Section "Microsoft Runtime (x86)" SecVCRuntime
  SetOutPath $TEMP
  File "vcredist_x86.exe"
  ExecWait '"$TEMP\vcredist_x86.exe" /q:a'
SectionEnd


Section -AdditionalIcons
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\bin\hox_Client.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\hox_Client.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function LaunchLink
  ExecShell "" "$SMPROGRAMS\${PRODUCT_NAME}\HOXChess.lnk"
FunctionEnd

;--------------------------------
; Assign descriptions to sections

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(S_SecMain)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecVCRuntime} "Microsoft Visual C++ 2008 Redistributable Package (x86)."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Installer Functions

Function .onInit
  Call CheckVCRedist
  Pop $R0
  ${if} $R0 != "-1"
      ; VS Retdist already installed. Default = [Not selected]
      SectionSetFlags ${SecVCRuntime} 0
  ${endif}

  ; Always show the language selection dialog,
  ; even if a language has been stored in the registry
  ;!define MUI_LANGDLL_ALWAYSSHOW

  !insertmacro MUI_LANGDLL_DISPLAY
  
  StrCpy $PRODUCT_LANGUAGE "default"
  StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
      StrCpy $PRODUCT_LANGUAGE "en_GB"
  StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +2
      StrCpy $PRODUCT_LANGUAGE "zh_CN"
  StrCmp $LANGUAGE ${LANG_VIETNAMESE} 0 +2
      StrCpy $PRODUCT_LANGUAGE "vi_VN"

  ;MessageBox MB_OK "Language = [$PRODUCT_LANGUAGE]"

  Call CheckAndUninstallPreviousVersion
FunctionEnd

;--------------------------------
; Uninstaller Section

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$TEMP\vcredist_x86.exe"
  Delete "$INSTDIR\README.txt"

  Delete "$INSTDIR\bin\hox_Client.exe"

  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk"
  Delete "$DESKTOP\HOXChess.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\HOXChess.lnk"

  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir /r "$INSTDIR\resource\locale\zh_CN"
  RMDir /r "$INSTDIR\resource\locale\vi_VN"
  RMDir "$INSTDIR\resource\locale"
  RMDir /r "$INSTDIR\resource\pieces\1"
  RMDir /r "$INSTDIR\resource\pieces\chessvariants_42x42"
  RMDir /r "$INSTDIR\resource\pieces\wikipedia_60x60"
  RMDir /r "$INSTDIR\resource\pieces\xqwizard_1_57x57"
  RMDir /r "$INSTDIR\resource\pieces\xqwizard_2_57x57"
  RMDir /r "$INSTDIR\resource\pieces\iXiangQi_55x55"
  RMDir "$INSTDIR\resource\pieces"
  RMDir /r "$INSTDIR\resource\images"
  RMDir /r "$INSTDIR\resource\sounds"
  RMDir "$INSTDIR\resource"
  RMDir /r "$INSTDIR\plugins"
  RMDir "$INSTDIR\plugins"
  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCU "${PRODUCT_CONFIG_REGKEY}"
  SetAutoClose true
SectionEnd

;--------------------------------
; Uninstaller Functions

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

;Function un.onUninstSuccess
;  HideWindow
;  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
;FunctionEnd

; --------------------------------------------------------------------------
; Detect whether your software is already installed
; and, if so, allows the user to uninstall it first.
;
; REFERENCE:
;  + http://nsis.sourceforge.net/Auto-uninstall_old_before_installing_new
;  + http://www.sraoss.jp/sylpheed/sylpheed/win32/nsis/sylpheed-2.2.3.nsi
; --------------------------------------------------------------------------
Function CheckAndUninstallPreviousVersion
  ReadRegStr $R0 HKLM ${PRODUCT_UNINST_KEY} "UninstallString"
  StrCmp $R0 "" done

  ; Build my own INSTDIR from the Uninstaller path
  ; since the default of INSTDIR = "../bin/.."
  StrLen $0 "uninst.exe"
  IntOp $0 $0 + 1
  IntOp $0 0 - $0
  StrCpy $1 $R0 $0
  ;MessageBox MB_OK "[$0] . Original = [$INSTDIR] My = [$R0] -> [$1]"
  StrCpy $INSTDIR $1    ; NOTE: Changed the Install Path
  
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(S_Found_Old)" IDOK uninst
  Abort

uninst:  ; Run the uninstaller
  ClearErrors

  Exec $R0
  ;ExecWait '$R0 _?=$INSTDIR' ; Do not copy the uninstaller to a temp file
  ;IfErrors no_remove_uninstaller
  ;no_remove_uninstaller:

done:
FunctionEnd

; --------------------------------------------------------------------------
; Test if Visual Studio Redistributables 2008+ (original) installed
; Returns -1 if there is no VC redistributables installed
;
; Reference: http://nsis.sourceforge.net/VC_8.0_Redistributables
; --------------------------------------------------------------------------
Function CheckVCRedist
   Push $R0
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}" "Version"

   ; if VC++2008 (original) redist not installed, install it
   IfErrors 0 VSRedistInstalled
   StrCpy $R0 "-1"

VSRedistInstalled:
   Exch $R0
FunctionEnd

;;;;;;;;;;;;;;;;;;;;; END OF FILE ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
