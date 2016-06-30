#-------------------------------------------------
#
# Project created by QtCreator 2016-06-13T14:58:26
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = FaceMapping
TEMPLATE = app


SOURCES +=\
    CPUT/source/directx/CPUT_DX11.cpp \
    CPUT/source/directx/CPUTAssetLibraryDX11.cpp \
    CPUT/source/directx/CPUTBufferDX11.cpp \
    CPUT/source/directx/CPUTComputeShaderDX11.cpp \
    CPUT/source/directx/CPUTDomainShaderDX11.cpp \
    CPUT/source/directx/CPUTGeometryShaderDX11.cpp \
    CPUT/source/directx/CPUTGPUTimerDX11.cpp \
    CPUT/source/directx/CPUTGuiControllerDX11.cpp \
    CPUT/source/directx/CPUTHullShaderDX11.cpp \
    CPUT/source/directx/CPUTInputLayoutCacheDX11.cpp \
    CPUT/source/directx/CPUTMaterialDX11.cpp \
    CPUT/source/directx/CPUTMeshDX11.cpp \
    CPUT/source/directx/CPUTPixelShaderDX11.cpp \
    CPUT/source/directx/CPUTPostProcess.cpp \
    CPUT/source/directx/CPUTRenderStateBlockDX11.cpp \
    CPUT/source/directx/CPUTRenderTarget.cpp \
    CPUT/source/directx/CPUTShaderDX11.cpp \
    CPUT/source/directx/CPUTTextureDX11.cpp \
    CPUT/source/directx/CPUTVertexShaderDX11.cpp \
    CPUT/source/windows/CPUTOSServicesWin.cpp \
    CPUT/source/windows/CPUTQtWindowWin.cpp \
    CPUT/source/windows/CPUTTimerWin.cpp \
    CPUT/source/windows/CPUTWindowWin.cpp \
    CPUT/source/CPUTAnimation.cpp \
    CPUT/source/CPUTAssetLibrary.cpp \
    CPUT/source/CPUTAssetSet.cpp \
    CPUT/source/CPUTButton.cpp \
    CPUT/source/CPUTCamera.cpp \
    CPUT/source/CPUTCheckbox.cpp \
    CPUT/source/CPUTConfigBlock.cpp \
    CPUT/source/CPUTControl.cpp \
    CPUT/source/CPUTDropdown.cpp \
    CPUT/source/CPUTFont.cpp \
    CPUT/source/CPUTFrustum.cpp \
    CPUT/source/CPUTGuiController.cpp \
    CPUT/source/CPUTITTTaskMarker.cpp \
    CPUT/source/CPUTLight.cpp \
    CPUT/source/CPUTMaterial.cpp \
    CPUT/source/CPUTMesh.cpp \
    CPUT/source/CPUTModel.cpp \
    CPUT/source/CPUTNullNode.cpp \
    CPUT/source/CPUTParser.cpp \
    CPUT/source/CPUTPerfTaskMarker.cpp \
    CPUT/source/CPUTRenderNode.cpp \
    CPUT/source/CPUTRenderStateBlock.cpp \
    CPUT/source/CPUTScene.cpp \
    CPUT/source/CPUTSkeleton.cpp \
    CPUT/source/CPUTSlider.cpp \
    CPUT/source/CPUTSoftwareMesh.cpp \
    CPUT/source/CPUTSoftwareTexture.cpp \
    CPUT/source/CPUTSprite.cpp \
    CPUT/source/CPUTText.cpp \
    CPUT/source/CPUTTexture.cpp \
    Extras/DirectXTex/DDSTextureLoader/DDSTextureLoader.cpp \
    Extras/DirectXTex/DirectXTex/BC.cpp \
    Extras/DirectXTex/DirectXTex/BC4BC5.cpp \
    Extras/DirectXTex/DirectXTex/BC6HBC7.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexCompress.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexConvert.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexD3D11.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexDDS.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexFlipRotate.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexImage.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexMipmaps.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexMisc.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexNormalMaps.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexPMAlpha.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexResize.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexTGA.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexUtil.cpp \
    Extras/DirectXTex/DirectXTex/DirectXTexWIC.cpp \
    Extras/DirectXTex/ScreenGrab/ScreenGrab.cpp \
    Extras/DirectXTex/Texconv/texconv.cpp \
    Extras/DirectXTex/WICTextureLoader/WICTextureLoader.cpp \
    facescan/Facemap/CDisplacementMapStage.cpp \
    facescan/Facemap/CHairGeometryStage.cpp \
    facescan/Facemap/CHeadBlendStage.cpp \
    facescan/Facemap/CHeadGeometryStage.cpp \
    facescan/Facemap/CPipeline.cpp \
    facescan/Facemap/FaceMapUtil.cpp \
    facescan/CFaceModel.cpp \
    facescan/OBJExporter.cpp \
    facescan/ObjLoader.cpp \
    facescan/SampleStart.cpp \
    facescan/windowsMain.cpp \
    CPUT/middleware/stb/stb_image.c \
    facescan/FaceMapping.cpp \
    facescan/FaceMappingUtil.cpp

HEADERS  +=  CPUT/include/android/CPUTTimerLinux.h \
    CPUT/include/android/CPUTWindowAndroid.h \
    CPUT/include/directx/CPUT_DX11.h \
    CPUT/include/directx/CPUTAssetLibraryDX11.h \
    CPUT/include/directx/CPUTBufferDX11.h \
    CPUT/include/directx/CPUTComputeShaderDX11.h \
    CPUT/include/directx/CPUTDomainShaderDX11.h \
    CPUT/include/directx/CPUTGeometryShaderDX11.h \
    CPUT/include/directx/CPUTGPUTimerDX11.h \
    CPUT/include/directx/CPUTGuiControllerDX11.h \
    CPUT/include/directx/CPUTHullShaderDX11.h \
    CPUT/include/directx/CPUTInputLayoutCacheDX11.h \
    CPUT/include/directx/CPUTMaterialDX11.h \
    CPUT/include/directx/CPUTMeshDX11.h \
    CPUT/include/directx/CPUTPixelShaderDX11.h \
    CPUT/include/directx/CPUTPostProcess.h \
    CPUT/include/directx/CPUTRenderStateBlockDX11.h \
    CPUT/include/directx/CPUTRenderStateMapsDX11.h \
    CPUT/include/directx/CPUTShaderDX11.h \
    CPUT/include/directx/CPUTTextureDX11.h \
    CPUT/include/directx/CPUTVertexShaderDX11.h \
    CPUT/include/windows/CPUTQtWindowWin.h \
    CPUT/include/windows/CPUTTimerWin.h \
    CPUT/include/windows/CPUTWindowWin.h \
    CPUT/include/windows/ThrowHResult.h \
    CPUT/include/CPUT.h \
    CPUT/include/CPUTAnimation.h \
    CPUT/include/CPUTAssetLibrary.h \
    CPUT/include/CPUTAssetLibrary.hpp \
    CPUT/include/CPUTAssetSet.h \
    CPUT/include/CPUTBuffer.h \
    CPUT/include/CPUTButton.h \
    CPUT/include/CPUTCallbackHandler.h \
    CPUT/include/CPUTCamera.h \
    CPUT/include/CPUTCheckbox.h \
    CPUT/include/CPUTConfigBlock.h \
    CPUT/include/CPUTControl.h \
    CPUT/include/CPUTCrossPlatform.h \
    CPUT/include/CPUTDropdown.h \
    CPUT/include/CPUTEventHandler.h \
    CPUT/include/CPUTFont.h \
    CPUT/include/CPUTFrustum.h \
    CPUT/include/CPUTGPUTimer.h \
    CPUT/include/CPUTGuiController.h \
    CPUT/include/CPUTInputLayoutCache.h \
    CPUT/include/CPUTITTTaskMarker.h \
    CPUT/include/CPUTLight.h \
    CPUT/include/CPUTMaterial.h \
    CPUT/include/CPUTMath.h \
    CPUT/include/CPUTMesh.h \
    CPUT/include/CPUTModel.h \
    CPUT/include/CPUTNullNode.h \
    CPUT/include/CPUTOSServices.h \
    CPUT/include/CPUTParser.h \
    CPUT/include/CPUTPerfTaskMarker.h \
    CPUT/include/CPUTRefCount.h \
    CPUT/include/CPUTRenderNode.h \
    CPUT/include/CPUTRenderParams.h \
    CPUT/include/CPUTRenderStateBlock.h \
    CPUT/include/CPUTRenderTarget.h \
    CPUT/include/CPUTResource.h \
    CPUT/include/CPUTScene.h \
    CPUT/include/CPUTSkeleton.h \
    CPUT/include/CPUTSlider.h \
    CPUT/include/CPUTSoftwareMesh.h \
    CPUT/include/CPUTSoftwareTexture.h \
    CPUT/include/CPUTSprite.h \
    CPUT/include/CPUTText.h \
    CPUT/include/CPUTTexture.h \
    CPUT/include/CPUTTimer.h \
    CPUT/include/CPUTWindow.h \
    Extras/DirectXTex/DDSTextureLoader/DDSTextureLoader.h \
    Extras/DirectXTex/DirectXTex/BC.h \
    Extras/DirectXTex/DirectXTex/DDS.h \
    Extras/DirectXTex/DirectXTex/DirectXTex.h \
    Extras/DirectXTex/DirectXTex/DirectXTexP.h \
    Extras/DirectXTex/DirectXTex/scoped.h \
    Extras/DirectXTex/ScreenGrab/ScreenGrab.h \
    Extras/DirectXTex/WICTextureLoader/WICTextureLoader.h \
    Extras/DirectXTex/XNAMath/xnamath.h \
    facescan/Facemap/CDisplacementMapStage.h \
    facescan/Facemap/CHairGeometryStage.h \
    facescan/Facemap/CHeadBlendStage.h \
    facescan/Facemap/CHeadGeometryStage.h \
    facescan/Facemap/CPipeline.h \
    facescan/Facemap/FaceMapUtil.h \
    facescan/FaceMapping.h \
    facescan/CFaceModel.h \
    facescan/OBJExporter.h \
    facescan/ObjLoader.h \
    facescan/resource.h \
    facescan/SampleStart.h \
    facescan/FaceMappingUtil.h

INCLUDEPATH += CPUT/include/DirectX \
    CPUT/include/windows \
    CPUT/include \
    Extras\DirectXTex\DDSTextureLoader \
    Extras\DirectXTex\WICTextureLoader \
    Extras\DirectXTex\XNAMath \
    Extras\DirectXTex\DirectXTex \
    Extras\DirectXTex\ScreenGrab \
    $$(RSSDK_DIR)/include

DEPENDPATH += $$(RSSDK_DIR)/include

win32: INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\8.1\Include\um"
win32: DEFINES += NOMINMAX CPUT_FOR_DX11 CPUT_OS_WINDOWS _UNICODE _CRT_SECURE_NO_WARNINGS

win32:CONFIG(release, debug|release): LIBS += -L$$(RSSDK_DIR)lib/Win32/ -llibpxc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(RSSDK_DIR)lib/Win32/ -llibpxc_d

LIBS += -lAdvapi32
LIBS += -lUser32
LIBS += -lOle32
LIBS += -lComdlg32
LIBS += -ld3d11
LIBS += -lD3DCompiler
LIBS += -ldxguid

win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$(RSSDK_DIR)lib/Win32/libpxc.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$(RSSDK_DIR)lib/Win32/libpxc_d.lib

QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
