// Copyright (C) 2020 averne
//
// This file is part of Fizeau.
//
// Fizeau is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Fizeau is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Fizeau.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdio>
#include <vector>
#include <string>
#include <switch.h>
#include <stratosphere.hpp>
#include <nvjpg.hpp>
#include <common.hpp>

#include "gfx.hpp"
#include "gui.hpp"

extern "C" void userAppInit() {
#ifdef DEBUG
#   ifdef TWILI
    twiliInitialize();
    twiliBindStdio();
#   else
    socketInitializeDefault();
    nxlinkStdio();
#   endif
#endif
    plInitialize(PlServiceType_User);
    romfsInit();
    appletLockExit();
    hidInitializeTouchScreen();
}

extern "C" void userAppExit(void) {
#ifdef DEBUG
#   ifdef TWILI
    twiliExit();
#   else
    socketExit();
#   endif
#endif
    romfsExit();
    plExit();
    appletUnlockExit();
}

fz::cfg::Config config;

int main(int argc, char **argv) {
    LOG("Starting Fizeau\n");

    if (R_FAILED(nj::initialize())) {
        LOG("Failed to init nvjpg");
        return 1;
    }
    NJ_SCOPEGUARD([] { nj::finalize(); });

    nj::Decoder decoder;
    if (auto rc = decoder.initialize(2); rc) {
        LOG("Failed to initialize decoder: %#x\n", rc);
        return 1;
    }
    NJ_SCOPEGUARD([&decoder] { decoder.finalize(); });

    nj::Image background("romfs:/background.jpg"), preview("romfs:/preview.jpg");
    if (!background.is_valid() || background.parse() || !preview.is_valid() || preview.parse()) {
        LOG("Invalid file");
        return 1;
    }

    nj::Surface background_surf(background.width, background.height, nj::PixelFormat::RGBA);
    nj::Surface preview_surf   (preview.width,    preview.height,    nj::PixelFormat::RGBA);
    if (R_FAILED(background_surf.allocate()) || R_FAILED(preview_surf.allocate())) {
        LOG("Failed to allocate surfaces\n");
        return 1;
    }

    if (R_FAILED(decoder.render(background, background_surf, 255)))
        LOG("Failed to render image\n");

    if (R_FAILED(decoder.render(preview, preview_surf, 255)))
        LOG("Failed to render image\n");

    if (!fz::gfx::init())
        LOG("Failed to init\n");

    decoder.wait(background_surf, preview_surf);

    dk::UniqueMemBlock background_memblk, preview_memblk;
    dk::Image background_img, preview_img;
    DkResHandle background_hdl = dkMakeTextureHandle(1, 1), preview_hdl = dkMakeTextureHandle(2, 2);

    fz::gfx::register_texture(background_memblk, background_img, background_surf, 1, 1);
    fz::gfx::register_texture(preview_memblk,    preview_img,    preview_surf,    2, 2);

    fz::gui::init();

    bool is_active;
    Result rc = fizeauIsServiceActive(&is_active);
    if (R_FAILED(rc) || !is_active) {
        LOG("Service not active: rc %#x, active %d\n", rc, is_active);
        rc = 1;
    }

    if (R_SUCCEEDED(rc))
        rc = fizeauInitialize();

    if (R_SUCCEEDED(rc))
        config = fz::cfg::read();

    if (R_SUCCEEDED(rc))
        rc = fz::cfg::open_profile(config, FizeauProfileId_Profile1);

    if (R_SUCCEEDED(rc))
        rc = fz::Clock::initialize();

    while (fz::gfx::loop()) {
        if (R_FAILED(rc)) {
            fz::gui::draw_error_window(config, rc);
            fz::gfx::render();
            continue;
        }

        fz::gui::draw_background(config, background_hdl);
        fz::gui::draw_preview_window(config, preview_hdl);
        fz::gui::draw_graph_window(config);
        rc = fz::gui::draw_main_window(config);

        static bool prev_editing_day = false;
        if (config.is_editing_day_profile && config.is_editing_night_profile) {
            if (prev_editing_day)
                config.is_editing_day_profile   = false, prev_editing_day = false;
            else
                config.is_editing_night_profile = false, prev_editing_day = true;
        }

        fz::gfx::render();
    }

    fz::cfg::dump(config);

    LOG("Exiting Fizeau\n");
    fizeauProfileClose(&config.cur_profile);
    fizeauExit();
    fz::gui::exit();

    background_memblk = nullptr, preview_memblk = nullptr;
    fz::gfx::exit();

    return 0;
}
