#ifndef FACEMAPPINGWIDGET_H
#define FACEMAPPINGWIDGET_H

#include "FaceMappingEngine.h"

class FaceMappingWidget: public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor blendColor1 MEMBER blendColor1 WRITE setBlendColor1)
    Q_PROPERTY(QColor blendColor2 MEMBER blendColor2 WRITE setBlendColor2)

    Q_PROPERTY(float postBMI MEMBER postBMI WRITE setPostBMI)


private:
    FaceMappingEngine* mFMEngine;
    QColor blendColor1;
    QColor blendColor2;
    float postBMI;

public:
    FaceMappingWidget();
    ~FaceMappingWidget();
    QSize sizeHint() const;

public slots:
    void setHairIndex(int idx);
    void loadFace(QString path);
    void storeHead(QString filename);

    void setMorphParamWeight(int idx, float weight);
    void setPostBMI(float weight);
    void setPostOgre(float weight);

    void setFaceZOffset(float offset);
    void setFaceOrientation(float yaw, float pitch, float roll);

    void setBlendColor1(QColor color);
    void setBlendColor2(QColor color);
};

#endif // FACEMAPPINGWIDGET_H
