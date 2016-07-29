#ifndef FACEMAPPINGWIDGET_H
#define FACEMAPPINGWIDGET_H

#include "FaceMappingEngine.h"
#include <QFuture>

class FaceMappingWidget: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor blendColor1 MEMBER blendColor1 WRITE setBlendColor1)
    Q_PROPERTY(QColor blendColor2 MEMBER blendColor2 WRITE setBlendColor2)

    Q_PROPERTY(float postBMI MEMBER postBMI WRITE setPostBMI)
    Q_PROPERTY(float postOgre MEMBER postOgre WRITE setPostOgre)

    Q_PROPERTY(float faceZOffset MEMBER faceZOffset WRITE setFaceZOffset)
    Q_PROPERTY(float faceOrientationYaw MEMBER faceOrientationYaw WRITE setFaceOrientationYaw)
    Q_PROPERTY(float faceOrientationPitch MEMBER faceOrientationPitch WRITE setFaceOrientationPitch)
    Q_PROPERTY(float faceOrientationRoll MEMBER faceOrientationRoll WRITE setFaceOrientationRoll)

    Q_PROPERTY(int hairIndex MEMBER hairIndex WRITE setHairIndex)

private:
    FaceMappingEngine* mFMEngine;
    bool mFMEngineTerminationRequired;
    QFuture<void> mFMEngineLoopResult;
    QColor blendColor1;
    QColor blendColor2;

    float postBMI;
    float postOgre;

    float faceZOffset = 0.0f;
    float faceOrientationYaw = 0.0f;
    float faceOrientationPitch = 0.0f;
    float faceOrientationRoll = 0.0f;

    int hairIndex = 0;

public:
    FaceMappingWidget();
    ~FaceMappingWidget();
    QSize sizeHint() const;

public slots:
    void setHairIndex(int idx);
    void setBeardIndex(int idx, bool enable=true);

    void startLoadingAssets();
    void startLoadingFace(QString path);
    void startExportingHead(QString filename);

    void setMorphParamWeight(int idx, float weight);
    void setPostBMI(float weight);
    void setPostOgre(float weight);

    void setFaceZOffset(float offset);
    void setFaceOrientation(float yaw, float pitch, float roll);
    void setFaceOrientationYaw(float yaw);
    void setFaceOrientationPitch(float pitch);
    void setFaceOrientationRoll(float roll);

    void setBlendColor1(QColor color);
    void setBlendColor2(QColor color);

    void startProcessingMessageLoop();
    void stopRenderingLoop();

signals:
    void assetsHaveLoaded();
    void faceHasLoaded();
    void headHasBeenExported(QString);
};

#endif // FACEMAPPINGWIDGET_H
