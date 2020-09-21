#include "valve_sdk/csgostructs.hpp"
#include "settings/settings.h"
#include "hooks/hooks.h"

namespace spy_camera
{
    ImVec2 window_size(250.f, 150.f);
    ITexture* view_texture;

    void on_render_view(CViewSetup& view)
    {
        if (!settings::misc::spy_camera)
            return;

        if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
            return;

        CViewSetup mirror_view = view;
        mirror_view.x = mirror_view.x_old = 0;
        mirror_view.y = mirror_view.y_old = 0;

        c_base_player* player = nullptr;

        for (int i = 1; i <= g::global_vars->maxClients; i++)
        {
            player = c_base_player::GetPlayerByIndex(i);

            if (player && player->IsAlive() && player->m_iTeamNum() != g::local_player->m_iTeamNum())
            {
                mirror_view.angles = player->m_angEyeAngles();
                mirror_view.origin = player->get_hitbox_position(player, HITBOX_HEAD);
            }
        }

        //mirror_view.origin.z -= 2.f; //z is down/up //was 15.f at chest area, was 10.f around viewmodel area
        //mirror_view.origin.x += 15.f; //x is forward/backward //+= -15.f = behind back //WORK IN PROGRESS
        //mirror_view.origin.y += 5.f;
        //mirror_view.origin.Normalized();

        mirror_view.m_flAspectRatio = float(mirror_view.width) / float(mirror_view.height);
        mirror_view.fov = 90.f;

        auto render_ctx = g::mat_system->GetRenderContext();

        render_ctx->PushRenderTargetAndViewport();
        render_ctx->SetRenderTarget(view_texture);

        hooks::render_view::original(g::view_render, mirror_view, mirror_view, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL, 0);

        render_ctx->PopRenderTargetAndViewport();
        render_ctx->Release();
    }

    void on_end_scene()
    {
        if (!view_texture || !settings::misc::spy_camera || !g::local_player)
            return;

        if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected())
            return;

        auto& style = ImGui::GetStyle();
        const auto old_padding = style.WindowPadding;
        style.WindowPadding = { 0.f, 0.f };

        ImGui::SetNextWindowSize(window_size);
        ImGui::SetNextWindowSizeConstraints({ 100.f, 20.f }, { 1000.f, 400.f });
        ImGui::Begin("Spy Camera", nullptr, ImGuiWindowFlags_NoTitleBar);
        {
            Texture_t* ptr = **reinterpret_cast<Texture_t***>(uintptr_t(view_texture) + 0x50);

            auto texture = *reinterpret_cast<IDirect3DTexture9**>(uintptr_t(ptr) + 0xC);

            window_size = ImGui::GetWindowSize();

            ImGui::Image(texture, window_size);
        }
        ImGui::End();

        style.WindowPadding = old_padding;
    }

    void on_fire_event()
    {
        g::mat_system->IsGameStarted() = false;
        {
            g::mat_system->BeginRenderTargetAllocation();

            view_texture = g::mat_system->CreateNamedRenderTargetTextureEx(g::mat_system->GetBackBufferFormat(), xorstr_("mirrorcam_rt"));

            g::mat_system->EndRenderTargetAllocation();
        }
        g::mat_system->IsGameStarted() = true;
    }

    void on_frame_render_start()
    {
        if (!g::local_player || !settings::misc::spy_camera)
            return;

        for (int k = 1; k < MAX_PLAYERS; k++)
        {
            if (k == g::local_player->GetIndex())
                continue;

            auto entity = g::entity_list->GetClientEntity(k);
            if (entity)
            {
                *(int*)(uintptr_t(entity) + 0xA30) = g::global_vars->framecount;
                *(int*)(uintptr_t(entity) + 0xA28) = 0;
            }
        }
    }
}