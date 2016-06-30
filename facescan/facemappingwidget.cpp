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

    loadFace(QCoreApplication::applicationDirPath() + QDir::separator() + "userdata" + QDir::separator() + "joe_sr300_1.obj");
}

QSize FaceMappingWidget::sizeHint() const{
    return QSize(640,480);
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
