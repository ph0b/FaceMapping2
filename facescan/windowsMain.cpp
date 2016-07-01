//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------

#include <QApplication>
#include "facemappingwidget.h"

#include <QPropertyAnimation>

#include <QTimer>

int main(int argc, char **argv)
{
    QApplication a(argc,argv);

    FaceMappingWidget fmw;
    fmw.show();

    //because we can!
    QPropertyAnimation animation(&fmw, "postBMI");
    animation.setDuration(2000);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);
    QObject::connect(&animation, &QPropertyAnimation::finished, [&]{
        if (animation.direction() == QPropertyAnimation::Backward) {
            animation.setDirection(QPropertyAnimation::Forward);
        } else {
            animation.setDirection(QPropertyAnimation::Backward);
        }
        animation.start();
    });

    QTimer::singleShot(0,[&] {
        fmw.loadFace(a.applicationDirPath() + "/userdata/joe_sr300_1.obj");

        fmw.setMorphParamWeight(FaceMappingWidget::MorphParamIndexes::Jaw_Level, 0.6f);

        //fmw.setBlendColor1(QColor(10,10,10));
        //fmw.setPostBMI(0.8);
        //fmw.setPostOgre(0.8);
        //fmw.setFaceOrientation(0.f,0.f,0.f);
        //fmw.setFaceZOffset(-3.5f);

        fmw.setHairIndex(FaceMappingWidget::HairIndexes::Helmet_Short);

        animation.start();

        //fmw.storeHead(a.applicationDirPath() + "/userdata/generated.obj");
    });

    return a.exec();
}
