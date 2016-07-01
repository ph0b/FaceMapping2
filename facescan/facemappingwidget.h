#ifndef FACEMAPPINGWIDGET_H
#define FACEMAPPINGWIDGET_H

#include "FaceMappingEngine.h"

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
    QColor blendColor1;
    QColor blendColor2;

    float postBMI;
    float postOgre;

    float faceZOffset;
    float faceOrientationYaw;
    float faceOrientationPitch;
    float faceOrientationRoll;

    int hairIndex;

public:
    FaceMappingWidget();
    ~FaceMappingWidget();
    QSize sizeHint() const;

    struct PostMorphParamIndexes { enum { Post_BMI=0, Post_Ogre}; };

    struct MorphParamIndexes { enum {  HeadWidth = 0, Eye_Area_Width, Cheekbone_Width,
                                       OCC_Width, Jaw_Width, Jaw_Level, Chin_Width, Neck_Width,
                                       Shape_1, Shape_2, Shape_3, Width, Roundness, BMI, Cheekbone_Size,
                                       Jaw_Chin_Protrude, Jaw_Chin_Level, Jaw_Neck_Slope,
                                       Jaw_Angle, Brow_Thickness, Brow_Height, Chin_Front,
                                       Chin_Level, Ovalness}; };

    struct HairIndexes {enum { HairLess = 0, Short, Medium, Long, Helmet_Short, Helmet_2, Helmet_3, Helmet_4};};


public slots:
    void setHairIndex(int idx);
    void loadFace(QString path);
    void storeHead(QString filename);

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
};

#endif // FACEMAPPINGWIDGET_H
