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

#include "covergenerator.h"
#include "safe_file_names.hpp"

#include <QMutexLocker>
#include <QImage>

#include <flacfile.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <tpropertymap.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <vorbisfile.h>

#define check_finished() if(_finished){return;}


bool CoverGenerator::clear_covers()
{
    QDir dir = QDir::home();
    if(!dir.cd(".cache"))
    {
        return false;
    }
    if(!dir.cd("media-art"))
    {
        return false;
    }

    bool success = true;
    foreach(QString file, dir.entryList(QDir::Files | QDir::NoDotAndDotDot))
    {
        if(!dir.remove(file))
        {
            success = false;
        }
    }
    return success;
}

CoverGenerator::CoverGenerator() :
    _finished(true),
    _recursive(true),
    _paths_mutex(),
    _paths()
{
}

void CoverGenerator::clear_list()
{
    _paths.clear();
}

void CoverGenerator::add_path(QString path)
{
    QMutexLocker locker(&_paths_mutex);
    _paths.insert(path);
}

void CoverGenerator::setRecursive(bool recursive)
{
    _recursive = recursive;
}

bool CoverGenerator::finished()
{
    return _finished;
}

bool CoverGenerator::recursive()
{
    return _recursive;
}

QSet<QString> CoverGenerator::paths()
{
    return _paths;
}

void CoverGenerator::abort()
{
    _finished = true;
}

void CoverGenerator::run()
{
    QMutexLocker locker(&_paths_mutex);
    QSet<QString> processed_cache;

    _finished = false;
    emit generation_started();

    QDir dir = QDir::home();
    if(!dir.cd(".cache"))
    {
        if(!(dir.mkdir(".cache") && dir.cd(".cache")))
        {
            _finished = true;
            emit generation_finished();
            return;
        }
    }
    if(!dir.cd("media-art"))
    {
        if(!(dir.mkdir("media-art") && dir.cd("media-art")))
        {
            _finished = true;
            emit generation_finished();
            return;
        }
    }

    for(QSet<QString>::iterator i = _paths.begin(); i != _paths.end(); ++i)
    {
        process_dir(*i, dir, processed_cache);
    }

    _finished = true;
    emit generation_finished();
}

QString CoverGenerator::get_cache_string(QString &artist, QString &album)
{
    return QString("%1|%2").arg(artist).arg(album);
}

void CoverGenerator::process_dir(QString dir, QDir &media_dir, QSet<QString> &processed_cache)
{
    check_finished();
    QDir work_dir(dir);
    if(!work_dir.exists())
    {
        return;
    }
    foreach(QString file, work_dir.entryList(QDir::Files | QDir::NoDotAndDotDot))
    {
        check_finished();

        if(!QFile::exists(work_dir.absoluteFilePath(file)))
        {
            continue;
        }

        // FLAC
        if(file.endsWith(".flac", Qt::CaseInsensitive) || file.endsWith(".fla", Qt::CaseInsensitive))
        {
            TagLib::FLAC::File flac(work_dir.absoluteFilePath(file).toLatin1().data());
            TagLib::List<TagLib::FLAC::Picture *> pictures = flac.pictureList();
            TagLib::PropertyMap tagmap = flac.properties();
            if(pictures.size() > 0)
            {
                QString artist = tagmap.contains("ARTIST") ? QString::fromStdString(tagmap["ARTIST"].toString().to8Bit(true)) : " ";
                QString album = tagmap.contains("ALBUM") ? QString::fromStdString(tagmap["ALBUM"].toString().to8Bit(true)) : " ";
                // Abort if we have already processed this artist/album combination
                if(processed_cache.contains(get_cache_string(artist, album)))
                {
                    continue;
                }
                QImage image = QImage::fromData(reinterpret_cast<const uchar *> (pictures[0]->data().data()), pictures[0]->data().size());
                if(!image.isNull())
                {
                    image = image.scaled(QSize(SCALED_SIZE,SCALED_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    image.save(media_dir.absoluteFilePath(SafeFileNames::get_file_name(artist, album)));
                    processed_cache.insert(get_cache_string(artist, album));
                }
            }
        }
        // MP3
        else if(file.endsWith(".mp3", Qt::CaseInsensitive))
        {
            TagLib::MPEG::File mp3(work_dir.absoluteFilePath(file).toLatin1().data());
            if(mp3.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = mp3.ID3v2Tag(false);
                if(tags->frameListMap()["APIC"].isEmpty())
                {
                    // No images
                    continue;
                }
                QString artist = " ";
                QString album = " ";
                if(tags->artist() != TagLib::String::null)
                {
                    artist = QString::fromStdString(tags->artist().to8Bit(true));
                }
                if(tags->album() != TagLib::String::null)
                {
                    album = QString::fromStdString(tags->album().to8Bit(true));
                }

                // Abort if we have already processed this artist/album combination
                if(processed_cache.contains(get_cache_string(artist, album)))
                {
                    continue;
                }

                TagLib::ID3v2::FrameList apic = tags->frameListMap()["APIC"];

                // Save first picture
                TagLib::ID3v2::AttachedPictureFrame * picture = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(apic[0]);

                // Try to find 'front cover' picture
                for(TagLib::ID3v2::FrameList::Iterator i = apic.begin(); i != apic.end(); ++i)
                {
                    TagLib::ID3v2::AttachedPictureFrame *test_picture = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*i);
                    if(test_picture != 0 && test_picture->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                    {
                        picture = test_picture;
                        break;
                    }
                }

                // Save image if not null
                if(picture != 0)
                {
                    QImage image = QImage::fromData(reinterpret_cast<const uchar *> (picture->picture().data()), picture->picture().size());
                    if(!image.isNull())
                    {
                        image = image.scaled(QSize(SCALED_SIZE,SCALED_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        image.save(media_dir.absoluteFilePath(SafeFileNames::get_file_name(artist, album)));
                        processed_cache.insert(get_cache_string(artist, album));
                    }
                }
            }
        }
        // OGG
        else if(file.endsWith(".ogg", Qt::CaseInsensitive) || file.endsWith(".oga", Qt::CaseInsensitive) || file.endsWith(".ogv", Qt::CaseInsensitive) || file.endsWith(".ogx", Qt::CaseInsensitive) || file.endsWith(".spx", Qt::CaseInsensitive) || file.endsWith(".opus", Qt::CaseInsensitive))
        {
            // We need to test for all codecs
            TagLib::Ogg::FLAC::File flac(work_dir.absoluteFilePath(file).toLatin1().data());
            TagLib::Ogg::Vorbis::File vorbis(work_dir.absoluteFilePath(file).toLatin1().data());
            TagLib::Ogg::Speex::File speex(work_dir.absoluteFilePath(file).toLatin1().data());
            TagLib::Ogg::Opus::File opus(work_dir.absoluteFilePath(file).toLatin1().data());
            TagLib::Ogg::XiphComment *comments = 0;
            if(flac.isValid())
            {
                // FLAC codec
                if(!flac.hasXiphComment())
                {
                    continue;
                }
                comments = flac.tag();
            }
            else if(vorbis.isValid())
            {
                // vorbis codec
                comments = vorbis.tag();
            }
            else if(speex.isValid())
            {
                // speex codec
                comments = speex.tag();
            }
            else if(opus.isValid())
            {
                // opus codec
                comments = opus.tag();
            }
            else
            {
                // No codec found - just continue with next file
                continue;
            }

            // Extract information
            if(!comments->contains("METADATA_BLOCK_PICTURE"))
            {
                // No image
                continue;
            }
            QString album = " ";
            QString artist = " ";
            if(comments->album() != TagLib::String::null)
            {
                album = QString::fromStdString(comments->album().to8Bit(true));
            }
            if(comments->artist() != TagLib::String::null)
            {
                artist = QString::fromStdString(comments->artist().to8Bit(true));
            }

            // Abort if we have already processed this artist/album combination
            if(processed_cache.contains(get_cache_string(artist, album)))
            {
                continue;
            }

            // Convert image data from base64
            QString base64_image = QString::fromStdString(comments->fieldListMap()["METADATA_BLOCK_PICTURE"].front().to8Bit());
            QByteArray image_data = QByteArray::fromBase64(base64_image.toLocal8Bit());
            TagLib::FLAC::Picture picture(TagLib::ByteVector(image_data.data(), image_data.size()));

            QImage image = QImage::fromData(reinterpret_cast<const uchar *> (picture.data().data()), picture.data().size());
            if(!image.isNull())
            {
                image = image.scaled(QSize(SCALED_SIZE,SCALED_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                image.save(media_dir.absoluteFilePath(SafeFileNames::get_file_name(artist, album)));
                processed_cache.insert(get_cache_string(artist, album));
            }
        }
    }
    if(_recursive)
    {
        foreach(QString subdir, work_dir.entryList(QDir::Dirs  | QDir::NoDotAndDotDot))
        {
            check_finished();
            process_dir(work_dir.absoluteFilePath(subdir), media_dir, processed_cache);
        }
    }
}

