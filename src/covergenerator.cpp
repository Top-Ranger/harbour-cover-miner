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

// FLAC
#include <flacfile.h>

// mp3
#include <mpegfile.h>
#include <attachedpictureframe.h>
#include <tpropertymap.h>

// ogg
#include <oggfile.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <vorbisfile.h>

// Trueaudio
#include <trueaudiofile.h>

// aiff
#include <aifffile.h>

// wav
#include <wavfile.h>

// mp4
#include <mp4file.h>

#define check_finished() if(_abort){return;}


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
    _abort(false),
    _finished(true),
    _recursive(true),
    _paths_mutex(),
    _paths()
{
}

void CoverGenerator::clear_list()
{
    QMutexLocker locker(&_paths_mutex);
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
    _abort = true;
}

void CoverGenerator::wait_finished()
{
    this->wait();
}

void CoverGenerator::run()
{
    QMutexLocker locker(&_paths_mutex);
    QSet<QString> paths = _paths;
    locker.unlock();
    QSet<QString> processed_cache;

    _finished = false;
    _abort = false;
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

    for(QSet<QString>::iterator i = paths.begin(); i != paths.end(); ++i)
    {
        // Check if we should abort early
        if(_abort)
        {
            break;
        }
        process_dir(*i, dir, processed_cache);
    }

    _finished = true;
    emit generation_finished();
}

QString CoverGenerator::get_cache_string(QString &artist, QString &album)
{
    return QString("%1|%2").arg(artist).arg(album);
}

void CoverGenerator::process_ID3v2(TagLib::ID3v2::Tag *tags, QSet<QString> &processed_cache, QDir &media_dir)
{
    if(tags->frameListMap()["APIC"].isEmpty())
    {
        // No images
        return;
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
        return;
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
            if(!flac.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
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
            if(!mp3.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            if(mp3.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = mp3.ID3v2Tag(false);
                process_ID3v2(tags, processed_cache, media_dir);
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
        // TrueAudio
        else if(file.endsWith(".tta", Qt::CaseInsensitive))
        {
            TagLib::TrueAudio::File tta(work_dir.absoluteFilePath(file).toLatin1().data());
            if(!tta.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            if(tta.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = tta.ID3v2Tag(false);
                process_ID3v2(tags, processed_cache, media_dir);
            }
        }
        // TrueAudio
        else if(file.endsWith(".tta", Qt::CaseInsensitive))
        {
            TagLib::TrueAudio::File tta(work_dir.absoluteFilePath(file).toLatin1().data());
            if(!tta.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            if(tta.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = tta.ID3v2Tag(false);
                process_ID3v2(tags, processed_cache, media_dir);
            }
        }
        // aiff
        else if(file.endsWith(".aiff", Qt::CaseInsensitive) || file.endsWith(".aif", Qt::CaseInsensitive) || file.endsWith(".aifc", Qt::CaseInsensitive))
        {
            TagLib::RIFF::AIFF::File aiff(work_dir.absoluteFilePath(file).toLatin1().data());
            if(!aiff.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            if(aiff.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = aiff.tag();
                process_ID3v2(tags, processed_cache, media_dir);
            }
        }
        // wav
        else if(file.endsWith(".wav", Qt::CaseInsensitive))
        {
            TagLib::RIFF::WAV::File wav(work_dir.absoluteFilePath(file).toLatin1().data());
            if(!wav.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            if(wav.hasID3v2Tag())
            {
                TagLib::ID3v2::Tag *tags = wav.tag();
                process_ID3v2(tags, processed_cache, media_dir);
            }
        }
        // mp4
        else if(file.endsWith(".mp4", Qt::CaseInsensitive) || file.endsWith(".m4a", Qt::CaseInsensitive))
        {
            TagLib::MP4::File mp4(work_dir.absoluteFilePath(file).toLatin1().data());
            if(!mp4.isValid())
            {
                // Invalid file - just skip it
                continue;
            }
            TagLib::MP4::Tag *tags = mp4.tag();
            if(tags == 0)
            {
                // No tags
                continue;
            }
            if(!tags->itemListMap().contains("covr"))
            {
                // No covers - continue
                continue;
            }
            QString album = " ";
            QString artist = " ";
            if(tags->album() != TagLib::String::null)
            {
                album = QString::fromStdString(tags->album().to8Bit(true));
            }
            if(tags->artist() != TagLib::String::null)
            {
                artist = QString::fromStdString(tags->artist().to8Bit(true));
            }
            // Abort if we have already processed this artist/album combination
            if(processed_cache.contains(get_cache_string(artist, album)))
            {
                continue;
            }

            // Find a cover image
            TagLib::MP4::CoverArtList pictures = tags->itemListMap()["covr"].toCoverArtList();
            for(TagLib::MP4::CoverArtList::ConstIterator i = pictures.begin(); i != pictures.end(); ++i)
            {
                TagLib::MP4::CoverArt cover = *i;
                QImage image = QImage::fromData(reinterpret_cast<const uchar *>(cover.data().data()), cover.data().size());
                if(!image.isNull())
                {
                    // Found valid picture
                    image = image.scaled(QSize(SCALED_SIZE,SCALED_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    image.save(media_dir.absoluteFilePath(SafeFileNames::get_file_name(artist, album)));
                    processed_cache.insert(get_cache_string(artist, album));
                    // No need to search furher
                    break;
                }
            }
        }
    }

    if(_recursive)
    {
        foreach(QString subdir, work_dir.entryList(QDir::Dirs  | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        {
            check_finished();
            process_dir(work_dir.absoluteFilePath(subdir), media_dir, processed_cache);
        }
    }
}
