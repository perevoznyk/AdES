
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "miniz.h"


namespace ZIPUTILS
	{

	// My class
	class ZIP
		{
		private:

			std::string fil;
			const char* mem = 0;
			size_t mems = 0;
		public:

	
			ZIP(const char* fi)
				{
				if (fi)
					fil = fi;
				}
			ZIP(const char* nmem,size_t nsz)
				{
				mem = nmem;
				mems = nsz;
				}



			HRESULT PutDirectory(const char* name)
				{
				if (!name)
					return E_INVALIDARG;
				if (!strlen(name))
					return E_INVALIDARG;
				std::string n = name;
				if (n[n.length() - 1] != '/')
					n += '/';
				if (!mz_zip_add_mem_to_archive_file_in_place(fil.c_str(), n.c_str(), 0, 0, 0, 0, MZ_BEST_COMPRESSION))
					return E_FAIL;
				return S_OK;
				}


			mz_bool InitFromReader(mz_zip_archive* a)
				{
				if (fil.length())
					return mz_zip_reader_init_file(a, fil.c_str(), 0);
				if (mem && mems)
					return mz_zip_reader_init_mem(a,mem,mems,0);
				return false;
				}


			template <typename T = unsigned char>
			HRESULT Extract(const char* fn, std::vector<T>& d)
				{
				mz_zip_archive zip_archive = { 0 };
				auto status = InitFromReader(&zip_archive);
				if (!status)
					return E_FAIL;

				size_t sz = 0;
				void* p = mz_zip_reader_extract_file_to_heap(&zip_archive, fn, &sz, 0);
				if (!p)
					{
					mz_zip_reader_end(&zip_archive);
					return E_FAIL;
					}
				if (p)
					{
					d.resize(sz);
					memcpy(d.data(), p, sz);
					mz_free(p);
					}
				mz_zip_reader_end(&zip_archive);
				return S_OK;
				}

			HRESULT ExtractToFile(const char* fn, const wchar_t* fi)
				{
				mz_zip_archive zip_archive = { 0 };
				auto status = InitFromReader(&zip_archive);
				if (!status)
					return E_FAIL;

				size_t sz = 0;
				void* p = mz_zip_reader_extract_file_to_heap(&zip_archive, fn, &sz, 0);
				if (!p)
					{
					mz_zip_reader_end(&zip_archive);
					return E_FAIL;
					}
				if (p)
					{
					HANDLE hX = CreateFile(fi, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
					if (hX == INVALID_HANDLE_VALUE)
						{
						mz_free(p);
						return E_FAIL;
						}
					DWORD A = 0;
					WriteFile(hX,(void*) p, (DWORD)sz, &A, 0);
					mz_free(p);
					CloseHandle(hX);
					if (sz != A)
						return E_FAIL;
					}
				mz_zip_reader_end(&zip_archive);
				return S_OK;
				}

			HRESULT EnumFiles(std::vector<mz_zip_archive_file_stat>& f)
				{
				mz_zip_archive zip_archive = { 0 };
				auto status = InitFromReader(&zip_archive);
				if (!status)
					return E_FAIL;

				for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++)
					{
					mz_zip_archive_file_stat file_stat;
					if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
						{
						mz_zip_reader_end(&zip_archive);
						return E_FAIL;
						}
					f.push_back(file_stat);
					}
				mz_zip_reader_end(&zip_archive);
				return S_OK;
				}

			HRESULT PutFile(const char* name,const char* data,unsigned long sz)
				{
				if (!mz_zip_add_mem_to_archive_file_in_place(fil.c_str(), name, data, sz, 0, 0, MZ_BEST_COMPRESSION))
					return E_FAIL;
				return S_OK;
				}

			template <typename T = unsigned char>
			static HRESULT MemCompress(const T* d, mz_ulong sz, std::vector<T>& r)
				{
				if (!d || !sz)
					return E_FAIL;
				auto sz2 = compressBound(sz);
				r.resize(sz2);
				mz_ulong cmp_len = sz2;
				auto cmp_status = compress((unsigned char*)r.data(), &cmp_len,(const unsigned char*)d, sz);
				if (cmp_status != Z_OK)
					return E_FAIL;
				r.resize(cmp_len);
				return S_OK;
				}

			template <typename T = unsigned char>
			static HRESULT MemUncompress(const T* d, mz_ulong sz, std::vector<T>& r)
				{
				if (!d || !sz)
					return E_FAIL;
				mz_ulong u_l = r.size();
				auto rv = uncompress((unsigned char*)r.data(), &u_l,(const unsigned char*) d, sz);
				if (rv != Z_OK)
					return E_FAIL;
				r.resize(u_l);
				return S_OK;
				}

		};
	}
