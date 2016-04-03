/*
Copyright (C) 2016 Marcus Soll

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.
*/

#include <sailfishapp.h>
#include <QtQuick>

#include "covergenerator.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc,argv);
    QQuickView *view = SailfishApp::createView();

    CoverGenerator generator;

    view->rootContext()->setContextProperty("generator", &generator);

    view->setSource(SailfishApp::pathTo("qml/harbour-cover-miner.qml"));
    view->show();
    return app->exec();
}
