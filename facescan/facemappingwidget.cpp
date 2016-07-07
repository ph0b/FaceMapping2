#include "facemappingwidget.h"
#include <QBoxLayout>
#include <QDir>
#include <QtConcurrent>

FaceMappingWidget::FaceMappingWidget(): mFMEngine(NULL), mFMEngineTerminationRequired(false) {
    setLayout(new QHBoxLayout);

    mFMEngine = new FaceMappingEngine();

    // window and device parameters
    CPUTWindowCreationParams params;
    CPUTResult result = mFMEngine->CPUTCreateWindowAndContext("FaceMapping Engine", params);
    ASSERT(CPUTSUCCESS(result), "CPUT Error creating window and context.");
    mFMEngine->Create();
    layout()->addWidget(&mFMEngine->GetQWidget());
}

void FaceMappingWidget::startLoadingAssets(){
    QtConcurrent::run([=] {
        // Initialize COM (needed for WIC)
        HRESULT hr = S_OK;
        if (FAILED(hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
        {
            wprintf(L"Failed to initialize COM (%08X)\n", hr);
            return 1;
        }

        mFMEngine->LoadContent();

        emit assetsHaveLoaded();
    });
}

void FaceMappingWidget::startProcessingMessageLoop(){
    mFMEngineLoopResult = QtConcurrent::run([=] {
        while(!this->mFMEngineTerminationRequired)
            this->mFMEngine->ProcessMessageLoopEvents();
    });
}

void FaceMappingWidget::stopRenderingLoop()
{
    mFMEngineTerminationRequired = true;
    mFMEngineLoopResult.waitForFinished();
}


QSize FaceMappingWidget::sizeHint() const{
    return QSize(640,800);
}

FaceMappingWidget::~FaceMappingWidget() {

    stopRenderingLoop();

    mFMEngine->ReleaseResources();
    mFMEngine->DeviceShutdown();

    delete mFMEngine;
}

void FaceMappingWidget::setHairIndex(int idx)
{
    mFMEngine->SetHairIndex(idx);
}

void FaceMappingWidget::setBeardIndex(int idx, bool enable)
{
    mFMEngine->SetBeardIndex(idx, enable);
}

void FaceMappingWidget::startLoadingFace(QString path)
{
    QtConcurrent::run([=] {

        mFMEngine->LoadFace(path.toStdString());
        //TODO: set default values, then call engine to compute adapted values to get, then set from widget (can even animate them).

        emit faceHasLoaded();
    });
}

void FaceMappingWidget::startExportingHead(QString filename){
    QtConcurrent::run([=] {
        mFMEngine->ExportOBJTo(filename.toStdString());
        emit headHasBeenExported(filename);
    });
}

void FaceMappingWidget::setMorphParamWeight(int idx, float weight)
{
    mFMEngine->SetMorphParamWeight(idx, weight);
}

void FaceMappingWidget::setPostBMI(float weight)
{
    if(mFMEngine->IsFaceLoaded())
    mFMEngine->SetPostMorphParamWeight(PostMorphParamIndexes::Post_BMI, weight);
}

void FaceMappingWidget::setPostOgre(float weight)
{
    mFMEngine->SetPostMorphParamWeight(PostMorphParamIndexes::Post_Ogre, weight);
}

void FaceMappingWidget::setFaceOrientation(float yaw, float pitch, float roll){
    mFMEngine->SetFaceOrientation(yaw, pitch, roll);
}

void FaceMappingWidget::setFaceOrientationYaw(float yaw){
    mFMEngine->SetFaceOrientation(yaw, faceOrientationPitch, faceOrientationRoll);
}

void FaceMappingWidget::setFaceOrientationPitch(float pitch){
    mFMEngine->SetFaceOrientation(faceOrientationYaw, pitch, faceOrientationRoll);
}

void FaceMappingWidget::setFaceOrientationRoll(float roll){
    mFMEngine->SetFaceOrientation(faceOrientationYaw, faceOrientationPitch, roll);
}

void FaceMappingWidget::setBlendColor1(QColor color)
{
    blendColor1 = color;
    mFMEngine->SetBlendColor1(color.redF(), color.greenF(), color.blueF());
}

void FaceMappingWidget::setBlendColor2(QColor color){
    blendColor2 = color;
    mFMEngine->SetBlendColor2(color.redF(), color.greenF(), color.blueF());
}

void FaceMappingWidget::setFaceZOffset(float offset){
    mFMEngine->SetFaceZOffset(offset);
}

