// D3D9VertexStream.cpp
// KlayGE D3D9顶点流类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了CopyToMemory (2005.7.24)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>

#include <algorithm>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>

namespace KlayGE
{
	D3D9VertexStream::D3D9VertexStream(VertexStreamType type, uint8_t sizeElement, uint8_t ElementsPerVertex, bool staticStream)
			: VertexStream(type, sizeElement, ElementsPerVertex),
				currentSize_(0), numVertices_(0), 
				staticStream_(staticStream)
	{
	}

	bool D3D9VertexStream::IsStatic() const
	{
		return staticStream_;
	}

	size_t D3D9VertexStream::NumVertices() const
	{
		return numVertices_;
	}

	void D3D9VertexStream::Assign(void const * src, size_t numVertices)
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		numVertices_ = numVertices;

		size_t const vertexSize(this->SizeOfElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices);

		if (currentSize_ < size)
		{
			currentSize_ = size;

			D3D9RenderEngine const & renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
			d3d_device_ = renderEngine.D3DDevice();

			IDirect3DVertexBuffer9* theBuffer;
			TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(size),
				this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC,
				0, D3DPOOL_DEFAULT, &theBuffer, NULL));
			buffer_ = MakeCOMPtr(theBuffer);
		}

		void* dest;
		TIF(buffer_->Lock(0, 0, &dest,
			D3DLOCK_NOSYSLOCK | (this->IsStatic() ? 0 : D3DLOCK_DISCARD)));

		uint8_t* destPtr(static_cast<uint8_t*>(dest));
		uint8_t const * srcPtr(static_cast<uint8_t const *>(src));

		std::copy(srcPtr, srcPtr + size, destPtr);

		buffer_->Unlock();
	}

	void D3D9VertexStream::CopyToMemory(void* data)
	{
		size_t const size(this->SizeOfElement() * this->ElementsPerVertex() * numVertices_);

		void* src;
		TIF(buffer_->Lock(0, 0, &src, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

		uint8_t* destPtr(static_cast<uint8_t*>(data));
		uint8_t const * srcPtr(static_cast<uint8_t const *>(src));

		std::copy(srcPtr, srcPtr + size, destPtr);

		buffer_->Unlock();
	}

	boost::shared_ptr<IDirect3DVertexBuffer9> D3D9VertexStream::D3D9Buffer() const
	{
		return buffer_;
	}

	void D3D9VertexStream::DoOnLostDevice()
	{
		size_t const vertexSize(this->SizeOfElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices_);

		IDirect3DVertexBuffer9* temp;
		TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(size), 0, 0, D3DPOOL_SYSTEMMEM, &temp, NULL));
		boost::shared_ptr<IDirect3DVertexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

		std::copy(src, src + size, dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
		currentSize_ = size;
	}
	
	void D3D9VertexStream::DoOnResetDevice()
	{
		BOOST_ASSERT(dynamic_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()) != NULL);

		D3D9RenderEngine const & renderEngine(static_cast<D3D9RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		d3d_device_ = renderEngine.D3DDevice();

		size_t const vertexSize(this->SizeOfElement() * this->ElementsPerVertex());
		size_t const size(vertexSize * numVertices_);

		IDirect3DVertexBuffer9* temp;
		TIF(d3d_device_->CreateVertexBuffer(static_cast<UINT>(size),
				this->IsStatic() ? 0 : D3DUSAGE_DYNAMIC,
				0, D3DPOOL_DEFAULT, &temp, NULL));
		boost::shared_ptr<IDirect3DVertexBuffer9> buffer = MakeCOMPtr(temp);

		uint8_t* src;
		uint8_t* dest;
		TIF(buffer_->Lock(0, 0, reinterpret_cast<void**>(&src), D3DLOCK_NOSYSLOCK));
		TIF(buffer->Lock(0, 0, reinterpret_cast<void**>(&dest), D3DLOCK_NOSYSLOCK));

		std::copy(src, src + size, dest);

		buffer->Unlock();
		buffer_->Unlock();

		buffer_ = buffer;
		currentSize_ = size;
	}
}
