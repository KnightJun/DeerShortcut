add_rules("mode.debug", "mode.release")
add_includedirs("include")

target("DeerShortcut")
    add_rules("qt.shared")
    add_frameworks("QtCore", "QtGui")
    add_defines("DeerShortcut_LIB")
    add_files("include/*.h")
    add_headerfiles("include/*.h")
    add_files("src/qxtglobalshortcut.cpp") 
    if is_plat("windows") then 
        add_files("src/qxtglobalshortcut_win.cpp") 
    elseif is_plat("macosx") then 
        add_files("src/qxtglobalshortcut_mac.cpp") 
    elseif is_plat("linux") then 
        add_files("src/qxtglobalshortcut_x11.cpp") 
    end

target("test")
    set_default(false)
    add_rules("qt.console")
    add_includedirs("include")
    add_deps("DeerShortcut")
    add_frameworks("QtCore", "QtGui")
    add_files("test/main.cpp")