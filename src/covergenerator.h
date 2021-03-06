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

#ifndef COVERGENERATOR_H
#define COVERGENERATOR_H

#include <QThread>
#include <QList>
#include <QString>
#include <QSet>
#include <QDir>
#include <QMutex>

#include <id3v2tag.h>

class CoverGenerator : public QThread
{
    Q_OBJECT
public:
    Q_INVOKABLE static bool clear_covers();

    CoverGenerator();
    Q_INVOKABLE void clear_list();
    Q_INVOKABLE void add_path(QString path);
    Q_INVOKABLE void setRecursive(bool recursive);
    Q_INVOKABLE bool finished();
    Q_INVOKABLE bool recursive();
    Q_INVOKABLE QSet<QString> paths();

signals:
    void generation_started();
    void generation_finished();

public slots:
    Q_INVOKABLE void abort();
    Q_INVOKABLE void wait_finished();

protected:
    void run();

private:
    static inline QString get_cache_string(QString &artist, QString &album);
    static void process_ID3v2(TagLib::ID3v2::Tag *tags, QSet<QString> &processed_cache, QDir &media_dir);
    void process_dir(QString dir, QDir &media_dir, QSet<QString> &processed_cache);

    static const int SCALED_SIZE = 500;

    bool _abort;
    bool _finished;
    bool _recursive;
    QMutex _paths_mutex;
    QSet<QString> _paths;
};

#endif // COVERGENERATOR_H
