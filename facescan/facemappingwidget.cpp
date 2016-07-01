#include "facemappingwidget.h"
#include <QBoxLayout>
#include <QDir>

FaceMappingWidget::FaceMappingWidget(){
    setLayout(new QHBoxLayout);

    mFMEngine = new FaceMappingEngine();

    // window and device parameters
    CPUTWindowCreationParams params;
    CPUTResult result = mFMEngine->CPUTCreateWindowAndContext("FaceMapping Engine", params);
    ASSERT(CPUTSUCCESS(result), "CPUT Error creating window and context.");
    mFMEngine->Create();
    layout()->addWidget(&mFMEngine->GetQWidget());

    mFMEngine->LoadContent();

    //TODO: set default values, then call engine to compute adapted values to get, then set from widget (can even animate them).
}

QSize FaceMappingWidget::sizeHint() const{
    return QSize(640,800);
}

FaceMappingWidget::~FaceMappingWidget() {
    mFMEngine->ReleaseResources();
    mFMEngine->DeviceShutdown();
    delete mFMEngine;
}

void FaceMappingWidget::setHairIndex(int idx)
{
    mFMEngine->SetHairIndex(idx);
}

void FaceMappingWidget::loadFace(QString path)
{
    mFMEngine->LoadFace(path.toStdString());
}

void FaceMappingWidget::storeHead(QString filename){
    mFMEngine->ExportOBJTo(filename.toStdString());
}

void FaceMappingWidget::setMorphParamWeight(int idx, float weight)
{
    mFMEngine->setMorphParamWeight(idx, weight);
}

void FaceMappingWidget::setPostBMI(float weight)
{
   mFMEngine->setPostMorphParamWeight(PostMorphParamIndexes::Post_BMI, weight);
}

void FaceMappingWidget::setPostOgre(float weight)
{
    mFMEngine->setPostMorphParamWeight(PostMorphParamIndexes::Post_Ogre, weight);
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

