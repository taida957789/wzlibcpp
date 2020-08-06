#include "FileMapping.h"
#include <assert.h>

FileMapping::FileMapping() : m_hFileMap(0), m_qwFileSize(0), m_dwGran(0), m_dwBlockBytes(0), m_pMapped(0), m_qwMappedOffset(0), m_qwMappedBlockSize(0)
{
#ifdef _DEBUG
	m_ReadCount = 0;
#endif

	// ���ϵͳ��������
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_dwGran = sysinfo.dwAllocationGranularity;
	m_dwBlockBytes = m_dwGran * 1000;
}

FileMapping::~FileMapping()
{
	Close();
}

bool FileMapping::Open(const wchar_t * path, ReadHint rhint)
{
	if(path)
	{
		// �ر�ԭ���ļ�
		Close();

		// ��һ�����ļ�
		HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
			rhint == RANDOM ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN,
			0);

		// �����ʧ��, �򷵻�
		if(hFile == INVALID_HANDLE_VALUE) return false;

		// ����ļ�����
		DWORD dwFileSizeHigh = 0;
		m_qwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
		m_qwFileSize |= (((uint64)dwFileSizeHigh) << 32);

		// �����ļ�ӳ��
		m_hFileMap = CreateFileMappingW(hFile, 0, PAGE_READONLY, 0, 0, 0);
		CloseHandle(hFile);

		// ӳ��ʧ���򷵻�
		if(m_hFileMap == 0) return false;

		return true;
	}
	return false;
}

void FileMapping::Close()
{
	Unmap();

	if(m_hFileMap)
	{
		CloseHandle(m_hFileMap);
		m_hFileMap = 0;
	}
}

void FileMapping::Unmap()
{
	if(m_pMapped)
	{
		UnmapViewOfFile(m_pMapped);
		m_pMapped = 0;
	}
}

void * FileMapping::Read(uint64 addr, size_t len, size_t * revlen)
{
	if(addr >= m_qwFileSize) return 0;

	void * pRetData = 0;

	// ����Ѿ�����ӳ��, ���ҷ��Ϸ�Χ, ��ֱ�ӷ���
	if(m_pMapped &&
		addr >= m_qwMappedOffset &&
		addr + len <= m_qwMappedOffset + m_qwMappedBlockSize)
	{
		pRetData = (char*)m_pMapped + (addr - m_qwMappedOffset);
	}
	else
	{
		// �����ӳ�������, ����ӳ��
		Unmap();

		// addr ����ǰ��Ҫӳ���ʵ�ʵ�ַ�Ĳ�
		uint64 qwDataOffset = addr % m_dwGran;
		// ��ǰ��Ҫӳ���ʵ�ʵ�ַ
		m_qwMappedOffset = addr - qwDataOffset;

		m_qwMappedBlockSize = m_qwFileSize - m_qwMappedOffset;
		if(m_qwMappedBlockSize > m_dwBlockBytes) m_qwMappedBlockSize = m_dwBlockBytes;

		m_pMapped = MapViewOfFile(m_hFileMap,
			FILE_MAP_READ,
			(DWORD)(m_qwMappedOffset >> 32),
			(DWORD)m_qwMappedOffset,
			(DWORD)m_qwMappedBlockSize);

		if(m_pMapped)
		{
#ifdef _DEBUG
			m_ReadCount++;
#endif
			pRetData = (char*)m_pMapped + qwDataOffset;
		}
	}

	if(revlen)
	{
		if(addr + len > m_qwMappedOffset + m_qwMappedBlockSize)
			len = m_qwMappedOffset + m_qwMappedBlockSize - addr;

		*revlen = len;
	}

	return pRetData;
}

size_t FileMapping::GetBlockBytes() const
{
	return m_dwBlockBytes;
}

uint64 FileMapping::GetFileLen() const
{
	return m_qwFileSize;
}
