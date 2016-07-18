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
    QObject::connect(&animation, &QPropertyAnimation::finished, &fmw, [&animation]{
        if (animation.direction() == QPropertyAnimation::Backward) {
            animation.setDirection(QPropertyAnimation::Forward);
        } else {
            animation.setDirection(QPropertyAnimation::Backward);
        }
        animation.start();
    });
    animation.start();


    QObject::connect(&fmw, &FaceMappingWidget::assetsHaveLoaded, &fmw, [&fmw]{
        fmw.startLoadingFace(QApplication::applicationDirPath() + "/userdata/joe_sr300_1.obj");
    });

    QObject::connect(&fmw, &FaceMappingWidget::faceHasLoaded, &fmw, [&fmw]{
        QTimer::singleShot(1000, &fmw, [&fmw] {
            fmw.setMorphParamWeight(FaceMappingWidget::MorphParamIndexes::Jaw_Level, 0.6f);
            fmw.setHairIndex(FaceMappingWidget::HairIndexes::Helmet_2);
            fmw.setBeardIndex(FaceMappingWidget::BeardIndexes::Moustache);
        });

//        QTimer::singleShot(10000, &fmw, [&fmw] {
//            fmw.startExportingHead(QApplication::applicationDirPath() + "/userdata/generated.obj");
//        });
    });

    fmw.startLoadingAssets();
    fmw.startProcessingMessageLoop();

    return a.exec();
}
