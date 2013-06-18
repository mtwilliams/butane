// This file is part of Butane. See README.md and LICENSE.md for details.
// Copyright (c) 2012 Michael Williams <devbug@bitbyte.ca>

#include <butane/graphics/d3d11/swap_chain.h>

#include <butane/application.h>
#include <butane/platforms/windows/window.h>
#include <butane/graphics/d3d11/render_device.h>
#include <butane/graphics/d3d11/render_target.h>

namespace butane {
  static Allocator& __allocator_initializer() {
    static ProxyAllocator allocator("swap chains", Allocators::heap());
    return allocator;
  }

  static const thread_safe::Static< Allocator >
    __ts_allocator(&__allocator_initializer);

  static Allocator& allocator() {
    return __ts_allocator();
  }

  D3D11SwapChain::D3D11SwapChain(
    Window* window,
    RenderTarget* render_target,
    const PixelFormat pixel_format,
    const uint32_t width,
    const uint32_t height,
    bool fullscreen,
    bool vertical_sync
  ) : butane::SwapChain(window, render_target, pixel_format, width, height, fullscreen, vertical_sync)
    , _interface(nullptr)
  {
  }

  D3D11SwapChain::~D3D11SwapChain()
  {
    if (_interface)
      _interface->Release();
    _interface = nullptr;
  }

  SwapChain* SwapChain::create(
    Window* window,
    const PixelFormat pixel_format,
    const uint32_t width,
    const uint32_t height,
    bool fullscreen,
    bool vertical_sync )
  {
    assert(window != nullptr);
    assert(width > 0);
    assert(height > 0);

    DXGI_SWAP_CHAIN_DESC scd;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.RefreshRate.Numerator = vertical_sync ? 1 : 0;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = dxgi_format_from_pixel_format(pixel_format);
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.OutputWindow = ((windows::Window*)window)->handle();
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D11SwapChain* swap_chain = make_new(D3D11SwapChain, allocator())(
      window, nullptr, pixel_format, width, height, false, vertical_sync);

    D3D11RenderDevice* render_device =
      ((D3D11RenderDevice*)Application::render_device());

    /* swap_chain->_interface = */ {
      const HRESULT hr = render_device->factory()->CreateSwapChain(
        render_device->device(), &scd, &swap_chain->_interface);
      if (FAILED(hr))
        fail("IDXGIFactory::CreateSwapChain failed, hr=%#08x", hr);
    }

    D3D11RenderTarget* render_target =
      make_new(D3D11RenderTarget, allocator())(nullptr);
    swap_chain->_render_target = (RenderTarget*)render_target;

    ID3D11Texture2D* buffer; {
      const HRESULT hr = swap_chain->_interface->GetBuffer(
        0, __uuidof(ID3D11Texture2D), (void**)&buffer);
      if (FAILED(hr))
        fail("IDXGISwapChain::GetBuffer failed, hr=%#08x", hr);
    }

    /* render_target->_view = */ {
      D3D11_RENDER_TARGET_VIEW_DESC rtvd;
      rtvd.Format = scd.BufferDesc.Format;
      rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      rtvd.Texture2D.MipSlice = 0;

      const HRESULT hr = render_device->device()->CreateRenderTargetView(
        (ID3D11Resource*)buffer, &rtvd, &render_target->_view);
      if (FAILED(hr))
        fail("ID3D11Device::CreateRenderTargetView failed, hr=%#08x", hr);
    }

    buffer->Release();

    window->set_on_resized_handler(
      (Window::OnResizedHandler)&D3D11SwapChain::on_window_resized, (void*)swap_chain);

    window->set_on_windowed_handler(
      (Window::OnWindowedHandler)&D3D11SwapChain::on_window_windowed, (void*)swap_chain);

    window->set_on_fullscreen_handler(
      (Window::OnFullscreenHandler)&D3D11SwapChain::on_window_fullscreen, (void*)swap_chain);

    if (fullscreen)
      swap_chain->set_fullscreen(fullscreen);

    return swap_chain;
  }

  void D3D11SwapChain::destroy()
  {
    make_delete(D3D11SwapChain, allocator(), this);
  }

  // void D3D11SwapChain::set_window(
  //   Window* window )
  // {}

  // void D3D11SwapChain::set_pixel_format(
  //   const PixelFormat pf )
  // {}

  void D3D11SwapChain::set_dimensions(
    const uint32_t width,
    const uint32_t height )
  {
    butane::SwapChain::set_dimensions(width, height);

    D3D11RenderDevice* render_device =
      ((D3D11RenderDevice*)Application::render_device());

    // Otherwise a reference to associated buffers might still be held.
    render_device->context()->ClearState();

    // Holds a reference to the the swap chain's buffer.
    _render_target->destroy();

    /* Resize it. */ {
      const HRESULT hr = _interface->ResizeBuffers(
        0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
      if (FAILED(hr))
        fail("IDXGISwapChain::ResizeBuffers failed, hr=#%08x", hr);
    }

    D3D11RenderTarget* render_target =
      make_new(D3D11RenderTarget, allocator())(nullptr);
    _render_target = (RenderTarget*)render_target;

    ID3D11Texture2D* buffer; {
      const HRESULT hr = _interface->GetBuffer(
        0, __uuidof(ID3D11Texture2D), (void**)&buffer);
      if (FAILED(hr))
        fail("IDXGISwapChain::GetBuffer failed, hr=%#08x", hr);
    }

    /* render_target->_view = */ {
      D3D11_RENDER_TARGET_VIEW_DESC rtvd;
      rtvd.Format = dxgi_format_from_pixel_format(pixel_format());
      rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      rtvd.Texture2D.MipSlice = 0;

      const HRESULT hr = render_device->device()->CreateRenderTargetView(
        (ID3D11Resource*)buffer, &rtvd, &render_target->_view);
      if (FAILED(hr))
        fail("ID3D11Device::CreateRenderTargetView failed, hr=%#08x", hr);
    }

    buffer->Release();
  }

  void D3D11SwapChain::set_fullscreen(
    bool fullscreen )
  {
    butane::SwapChain::set_fullscreen(fullscreen);
    window()->set_fullscreen(fullscreen);
  }

  void D3D11SwapChain::set_vertical_sync(
    bool vertical_sync )
  {
    butane::SwapChain::set_vertical_sync(vertical_sync);

    D3D11RenderDevice* render_device =
      ((D3D11RenderDevice*)Application::render_device());

    // Otherwise a reference to associated buffers might still be held.
    render_device->context()->ClearState();

    // Holds a reference to the swap chain's buffer.
    _render_target->destroy();

    DXGI_SWAP_CHAIN_DESC scd;
    scd.BufferDesc.Width = width();
    scd.BufferDesc.Height = height();
    scd.BufferDesc.RefreshRate.Numerator = vertical_sync ? 1 : 0;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = dxgi_format_from_pixel_format(pixel_format());
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.OutputWindow = ((windows::Window*)window())->handle();
    scd.Windowed = fullscreen();
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    /* _interface = */ {
      if (_interface)
        _interface->Release();
      const HRESULT hr = render_device->factory()->CreateSwapChain(
        render_device->device(), &scd, &_interface);
      if (FAILED(hr))
        fail("IDXGIFactory::CreateSwapChain failed, hr=%#08x", hr);
    }

    D3D11RenderTarget* render_target =
      make_new(D3D11RenderTarget, allocator())(nullptr);
    _render_target = (RenderTarget*)render_target;

    ID3D11Texture2D* buffer; {
      const HRESULT hr = _interface->GetBuffer(
        0, __uuidof(ID3D11Texture2D), (void**)&buffer);
      if (FAILED(hr))
        fail("IDXGISwapChain::GetBuffer failed, hr=%#08x", hr);
    }

    /* render_target->_view = */ {
      D3D11_RENDER_TARGET_VIEW_DESC rtvd;
      rtvd.Format = dxgi_format_from_pixel_format(pixel_format());
      rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      rtvd.Texture2D.MipSlice = 0;

      const HRESULT hr = render_device->device()->CreateRenderTargetView(
        (ID3D11Resource*)buffer, &rtvd, &render_target->_view);
      if (FAILED(hr))
        fail("ID3D11Device::CreateRenderTargetView failed, hr=%#08x", hr);
    }

    buffer->Release();
  }

  void D3D11SwapChain::on_window_resized(
    D3D11SwapChain* swap_chain,
    Window* window )
  {
    assert(swap_chain != nullptr);
    assert(window != nullptr);
    assert(swap_chain->window() == window);

    swap_chain->set_dimensions(window->width(), window->height());
  }

  void D3D11SwapChain::on_window_windowed(
    D3D11SwapChain* swap_chain,
    Window* window )
  {
    assert(swap_chain != nullptr);
    assert(window != nullptr);
    assert(swap_chain->window() == window);

    const HRESULT hr = swap_chain->_interface->SetFullscreenState(FALSE, NULL);
    if (FAILED(hr))
      fail("IDXGISwapChain::SetFullscreenState failed, hr=%#08x", hr);
  }

  void D3D11SwapChain::on_window_fullscreen(
    D3D11SwapChain* swap_chain,
    Window* window )
  {
    assert(swap_chain != nullptr);
    assert(window != nullptr);
    assert(swap_chain->window() == window);

    const HRESULT hr = swap_chain->_interface->SetFullscreenState(TRUE, NULL);
    if (FAILED(hr))
      fail("IDXGISwapChain::SetFullscreenState failed, hr=%#08x", hr);
  }
} // butane
