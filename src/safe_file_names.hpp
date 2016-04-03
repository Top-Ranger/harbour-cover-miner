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

#ifndef SAFE_FILE_NAMES_HPP
#define SAFE_FILE_NAMES_HPP

#include <QString>
#include <QChar>
#include <QList>
#include <QCryptographicHash>

namespace SafeFileNames {

static QList<QChar> unwanted_chars;
static bool initialised = false;

inline QString compute_md5(QString s)
{
    QByteArray hash = QCryptographicHash::hash(s.toLocal8Bit(), QCryptographicHash::Md5);
    return QString(hash.toHex());
}

inline QString strip_name(QString s)
{
    if(!initialised)
    {
        // Forbidden characters - see https://wiki.gnome.org/action/show/DraftSpecs/MediaArtStorageSpec
        initialised = true;
        unwanted_chars.append(QChar(0x0029)); // )
        unwanted_chars.append(QChar(0x005f)); // _
        unwanted_chars.append(QChar(0x007b)); // {
        unwanted_chars.append(QChar(0x007d)); // }
        unwanted_chars.append(QChar(0x005b)); // [
        unwanted_chars.append(QChar(0x005d)); // ]
        unwanted_chars.append(QChar(0x0021)); // !
        unwanted_chars.append(QChar(0x0040)); // @
        unwanted_chars.append(QChar(0x0023)); // #
        unwanted_chars.append(QChar(0x0024)); // $
        unwanted_chars.append(QChar(0x005e)); // ^
        unwanted_chars.append(QChar(0x0026)); // &
        unwanted_chars.append(QChar(0x002a)); // *
        unwanted_chars.append(QChar(0x002b)); // +
        unwanted_chars.append(QChar(0x003d)); // =
        unwanted_chars.append(QChar(0x007c)); // |
        unwanted_chars.append(QChar(0x005c)); // '\'
        unwanted_chars.append(QChar(0x002f)); // /
        unwanted_chars.append(QChar(0x0022)); // "
        unwanted_chars.append(QChar(0x0027)); // '
        unwanted_chars.append(QChar(0x003f)); // ?
        unwanted_chars.append(QChar(0x003c)); // <
        unwanted_chars.append(QChar(0x003e)); // >
        unwanted_chars.append(QChar(0x007e)); // ~
        unwanted_chars.append(QChar(0x0060)); // `
    }

    // Remove text in braces
    // ()
    while(s.contains('('))
    {
        int start = s.indexOf('(');
        int end = s.indexOf(')');
        if(end == -1)
        {
            break;
        }
        s.remove(start, end-start+1);
    }

    // {}
    while(s.contains('{'))
    {
        int start = s.indexOf('{');
        int end = s.indexOf('}');
        if(end == -1)
        {
            break;
        }
        s.remove(start, end-start+1);
    }

    // []
    while(s.contains('['))
    {
        int start = s.indexOf('[');
        int end = s.indexOf(']');
        if(end == -1)
        {
            break;
        }
        s.remove(start, end-start+1);
    }

    // <>
    while(s.contains('<'))
    {
        int start = s.indexOf('<');
        int end = s.indexOf('>');
        if(end == -1)
        {
            break;
        }
        s.remove(start, end-start+1);
    }


    // Remove unwanted characters
    for(QList<QChar>::iterator i = unwanted_chars.begin(); i != unwanted_chars.end(); ++i)
    {
        s.remove(*i);
    }

    // Whitespaces
    s = s.trimmed();
    while(s.contains("  "))
    {
        s.replace("  ", " ");
    }

    // Tabs -> spaces
    s.replace("\t", " ");

    // Normalise
    s = s.normalized(QString::NormalizationForm_KD);
    return s;
}

inline QString get_file_name(QString artist, QString album)
{
    artist = compute_md5(strip_name(artist).toLower());
    album = compute_md5(strip_name(album).toLower());
    return QString("album-%1-%2.jpeg").arg(artist).arg(album);
}
}

#endif // SAFE_FILE_NAMES_HPP

