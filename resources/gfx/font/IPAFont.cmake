include_guard(GLOBAL)

function(ipa_build_font_image)
    ctr_add_graphics_target(IPA_TEXTURES_FONT ATLAS
            INPUTS
            resources/gfx/font/space.png
            resources/gfx/font/excel.png
            resources/gfx/font/quote.png
            resources/gfx/font/hash.png
            resources/gfx/font/dolar.png
            resources/gfx/font/percent.png
            resources/gfx/font/amp.png
            resources/gfx/font/apos.png
            resources/gfx/font/open_paren.png
            resources/gfx/font/close_paren.png
            resources/gfx/font/asterix.png
            resources/gfx/font/plus.png
            resources/gfx/font/comma.png
            resources/gfx/font/dash.png
            resources/gfx/font/dot.png
            resources/gfx/font/slash.png
            resources/gfx/font/0.png
            resources/gfx/font/1.png
            resources/gfx/font/2.png
            resources/gfx/font/3.png
            resources/gfx/font/4.png
            resources/gfx/font/5.png
            resources/gfx/font/6.png
            resources/gfx/font/7.png
            resources/gfx/font/8.png
            resources/gfx/font/9.png
            resources/gfx/font/colon.png
            resources/gfx/font/semicolon.png
            resources/gfx/font/angle_open.png
            resources/gfx/font/equal.png
            resources/gfx/font/angle_close.png
            resources/gfx/font/question.png
            resources/gfx/font/at.png
            resources/gfx/font/A_C.png
            resources/gfx/font/B_C.png
            resources/gfx/font/C_C.png
            resources/gfx/font/D_C.png
            resources/gfx/font/E_C.png
            resources/gfx/font/F_C.png
            resources/gfx/font/G_C.png
            resources/gfx/font/H_C.png
            resources/gfx/font/I_C.png
            resources/gfx/font/J_C.png
            resources/gfx/font/K_C.png
            resources/gfx/font/L_C.png
            resources/gfx/font/M_C.png
            resources/gfx/font/N_C.png
            resources/gfx/font/O_C.png
            resources/gfx/font/P_C.png
            resources/gfx/font/Q_C.png
            resources/gfx/font/R_C.png
            resources/gfx/font/S_C.png
            resources/gfx/font/T_C.png
            resources/gfx/font/U_C.png
            resources/gfx/font/V_C.png
            resources/gfx/font/W_C.png
            resources/gfx/font/X_C.png
            resources/gfx/font/Y_C.png
            resources/gfx/font/Z_C.png
            resources/gfx/font/open_bracket.png
            resources/gfx/font/back_slash.png
            resources/gfx/font/close_bracket.png
            resources/gfx/font/caret.png
            resources/gfx/font/_.png
            resources/gfx/font/grave.png
            resources/gfx/font/a.png
            resources/gfx/font/b.png
            resources/gfx/font/c.png
            resources/gfx/font/d.png
            resources/gfx/font/e.png
            resources/gfx/font/f.png
            resources/gfx/font/g.png
            resources/gfx/font/h.png
            resources/gfx/font/i.png
            resources/gfx/font/j.png
            resources/gfx/font/k.png
            resources/gfx/font/l.png
            resources/gfx/font/m.png
            resources/gfx/font/n.png
            resources/gfx/font/o.png
            resources/gfx/font/p.png
            resources/gfx/font/q.png
            resources/gfx/font/r.png
            resources/gfx/font/s.png
            resources/gfx/font/t.png
            resources/gfx/font/u.png
            resources/gfx/font/v.png
            resources/gfx/font/w.png
            resources/gfx/font/x.png
            resources/gfx/font/y.png
            resources/gfx/font/z.png
            resources/gfx/font/brace_open.png
            resources/gfx/font/bar.png
            resources/gfx/font/brace_close.png
            resources/gfx/font/tilde.png
            resources/gfx/font/unknown.png

            OPTIONS -f a4
    )

    dkp_install_assets(${PROJECT_NAME}_romfs
            DESTINATION resources/gfx
            TARGETS IPA_TEXTURES_FONT
    )

    set(input "resources/gfx/font/font_info.txt")
    dkp_resolve_file(input "${input}")
    add_custom_target(IPA_FONT_INFO ALL
            DEPENDS "${input}"
    )
    dkp_set_target_file(IPA_FONT_INFO
            "${input}"
    )
    dkp_install_assets(${PROJECT_NAME}_romfs
            DESTINATION resources/gfx
            TARGETS IPA_FONT_INFO
    )
endfunction()
