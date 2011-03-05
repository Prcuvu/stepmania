#include "global.h"

#include "ExportPackage.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "SpecialFiles.h"
#include "RageLog.h"
#include "Song.h"
#include "FileDownload.h"
#include "CreateZip.h"
#include "ScreenPrompt.h"
#include "FileDownload.h"
#include "XmlFile.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "Json/Value.h"
#include "JsonUtil.h"

static RString ReplaceInvalidFileNameChars( RString sOldFileName )
{
	RString sNewFileName = sOldFileName;
	const char charsToReplace[] = { 
		' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
		'+', '=', '[', ']', '{', '}', '|', ':', '\"', '\\',
		'<', '>', ',', '?', '/' 
	};
	for( unsigned i=0; i<sizeof(charsToReplace); i++ )
		sNewFileName.Replace( charsToReplace[i], '_' );
	return sNewFileName;
}

void StripIgnoredSmzipFiles( vector<RString> &vsFilesInOut )
{
	for( int i=vsFilesInOut.size()-1; i>=0; i-- )
	{
		const RString &sFile = vsFilesInOut[i];

		bool bEraseThis = false;
		bEraseThis |= EndsWith( sFile, "smzip.ctl" );
		bEraseThis |= EndsWith( sFile, ".old" );
		bEraseThis |= EndsWith( sFile, "Thumbs.db" );
		bEraseThis |= EndsWith( sFile, ".DS_Store" );
		bEraseThis |= (sFile.find("CVS") != string::npos);
		bEraseThis |= (sFile.find(".svn") != string::npos);

		if( bEraseThis )
			vsFilesInOut.erase( vsFilesInOut.begin()+i );
	}
}

bool ExportDir( RString sSmzipFile, RString sDirToExport, RString &sErrorOut )
{
	RageFile f;
	// TODO: Mount Desktop/ for each OS
	if( !f.Open(sSmzipFile, RageFile::WRITE) )
	{
		sErrorOut = ssprintf( "Couldn't open %s for writing: %s", sSmzipFile.c_str(), f.GetError().c_str() );
		return false;
	}

	CreateZip zip;
	zip.Start(&f);

	vector<RString> vs;
	GetDirListingRecursive( sDirToExport, "*", vs );
	StripIgnoredSmzipFiles( vs );
	FOREACH( RString, vs, s )
	{
		if( !zip.AddFile( *s ) )
		{
			sErrorOut = ssprintf( "Couldn't add file: %s", s->c_str() );
			return false;
		}
	}

	if( !zip.Finish() )
	{
		sErrorOut = ssprintf( "Couldn't write to file %s: %s", sSmzipFile.c_str(), f.GetError().c_str() );
		return false;
	}
	
	return true;
}

RString ExportSong( const Song *pSong )
{
	RString sDirToExport = pSong->GetSongDir();

	RString sPackageName = sDirToExport;
	TrimLeft( sPackageName, "/" );
	sPackageName = ReplaceInvalidFileNameChars( sPackageName + ".smzip" );

	RString sSmzipFile = SpecialFiles::DESKTOP_DIR + sPackageName;

	RString sErrorOut;
	if( !ExportDir(sSmzipFile, sDirToExport, sErrorOut) )
		return "Failed to export '" + sDirToExport + "' to '" + sSmzipFile + "'";

	return "Exported as '" + sSmzipFile + "'";
}

void ExportPackage::ExportSongWithUI( const Song *pSong )
{
	RString sResult = ExportSong( pSong );
	ScreenPrompt::Prompt( SM_None, sResult );
}

/*
 * (c) 2010 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
