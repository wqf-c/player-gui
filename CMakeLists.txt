cmake_minimum_required(VERSION 3.8)

#add_compile_options(-std=c++11)
project(player-gui VERSION 0.0.1)
set(APP_NAME ${PROJECT_NAME})
#set(INST_DIR "F:/gitRepos/gitea/kankan/build/_CPack_Packages/win64/NSIS/kankan-0.0.1-win64")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")
if (MSVC) # Check if we are using the Visual Studio compiler
	SET(CMAKE_CXX_FLAGS "/EHsc /utf-8")  # deal with MSVC Warning C4530
endif (MSVC)
set(project_needed_libraries)
set(project_needed_includes)
set(project_need_compile_file)


include(./.SET_LOCAL_PATH.cmake) #!!!可以覆盖掉上面的库的路径!!!

add_definitions(-DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_)

# 0. ========================define function========================
# 与项目的inlcude文件无关的测试
macro(add_execute_test file name)
	add_executable(${name} ${file})


	target_link_libraries(${name}
			PRIVATE
			${project_needed_libraries}
			)
endmacro()

# 与项目的include 和 src 文件夹有关的测试
macro(add_execute file name ispack)
	if (${ispack} STREQUAL true)
		add_executable(${name} MACOSX_BUNDLE ${file}
				${project_need_compile_file})
	else()
		add_executable(${name} ${file}
				${project_need_compile_file})
	endif ()

	target_include_directories(${name}
			PRIVATE
			${project_needed_includes}
			)

	target_link_libraries(${name}
			PRIVATE
			${project_needed_libraries}
			)
endmacro()

if (APPLE)
	set(AEC3_LIBRARY "${PROJECT_SOURCE_DIR}/lib/libns_lib.a")
else()
	set(AEC3_LIBRARY "${PROJECT_SOURCE_DIR}/lib/aec3_lib.lib")
endif ()


macro(add_compile_dir dirName)
	aux_source_directory(${PROJECT_SOURCE_DIR}/${dirName} project_compile_${dirName})
	message(project_compile_${dirName} = ${project_compile_${dirName}})
	set(project_need_compile_file ${project_need_compile_file} ${project_compile_${dirName}})
endmacro()

# 1. ========================查找所需的库========================

# 1.1 WS2_LIBRARY
if (WIN32) # Check if we are on Windows
	find_library(WS2_LIBRARY ws2_32)
endif ()

# 1.2 wxwidgets
if(WIN32)
	find_path(WXWIDGET_INCLUDE_DIR msvc/wx/setup.h)
	find_path(WXWIDGET_LIBRARY_DIR vc_x64_lib)
	link_directories(
		${WXWIDGET_LIBRARY_DIR}/vc_x64_lib/
	)
	set(wxWidgets_LIBRARIES
			wxbase31u_net.lib wxbase31u_xml.lib wxexpat.lib wxjpeg.lib wxmsw31u_adv.lib wxmsw31u_aui.lib wxmsw31u_core.lib wxmsw31u_gl.lib wxmsw31u_html.lib wxmsw31u_media.lib wxmsw31u_propgrid.lib wxmsw31u_qa.lib wxmsw31u_ribbon.lib wxmsw31u_richtext.lib wxmsw31u_stc.lib wxmsw31u_webview.lib wxmsw31u_xrc.lib wxpng.lib wxregexu.lib wxscintilla.lib wxtiff.lib wxzlib.lib
			winmm.lib
			comctl32.lib
			rpcrt4.lib
			wsock32.lib
			wininet.lib
	)
else()
	find_package(wxWidgets REQUIRED COMPONENTS net core base)
	include(${wxWidgets_USE_FILE})
endif()

######################################
#  FFmpeg lib setting
######################################
#make sure ffmpeg/bin is in your PATH.

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

#find_path(POSTPROC_INCLUDE_DIR libpostproc/postprocess.h)
#find_library(POSTPROC_LIBRARY postproc)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY swresample)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

######################################
#  openal lib setting
######################################

find_library(OPENAL32_LIBRARY OpenAL32)

find_path(AL_INCLUDE_DIR al.h)


if (APPLE)
	set(ECHO_LIBRARY "${PROJECT_SOURCE_DIR}/lib/libspeex_lib.a")
else()
	set(ECHO_LIBRARY "${PROJECT_SOURCE_DIR}/lib/aec3_lib.lib")
endif ()



# 2. ========================link and include========================
set(project_needed_includes
		${project_needed_includes}
		${PROJECT_SOURCE_DIR}/AEC3/include
		${PROJECT_SOURCE_DIR}/include/ui
		${PROJECT_SOURCE_DIR}/include/media
		${PROJECT_SOURCE_DIR}/include/utils
		${PROJECT_SOURCE_DIR}/include/test
		${PROJECT_SOURCE_DIR}/lib
		${WXWIDGET_INCLUDE_DIR}
		${WXWIDGET_INCLUDE_DIR}/msvc
		${JRTPLIB_INCLUDE_DIRS}
		${AVCODEC_INCLUDE_DIR}
		${AVFORMAT_INCLUDE_DIR} 
		${AVUTIL_INCLUDE_DIR} 
		${AVDEVICE_INCLUDE_DIR}
		${AVFILTER_INCLUDE_DIR}
#		${POSTPROC_INCLUDE_DIR}
		${SWRESAMPLE_INCLUDE_DIR}
		${SWSCALE_INCLUDE_DIR}
		${AL_INCLUDE_DIR}
		)

set(project_needed_libraries
		${project_needed_libraries}
		${wxWidgets_LIBRARIES}
		${WS2_LIBRARY}
		${JRTPLIB_LIBRARIES}
		${AVCODEC_LIBRARY}
		${AVFORMAT_LIBRARY}
		${AVUTIL_LIBRARY}
		${AVDEVICE_LIBRARY}
		${AVFILTER_LIBRARY}
#		${POSTPROC_LIBRARY}
		${SWRESAMPLE_LIBRARY}
		${SWSCALE_LIBRARY}
		${OPENAL32_LIBRARY}
		${ECHO_LIBRARY}
		)

# 3.========================打印相关信息========================
message(=========== 1.w32 ==============)
message(WS2_LIBRARY=${WS2_LIBRARY})

message(=========== 2.wxwidgets ==============)
if(WIN32)
message(WXWIDGET_INCLUDE_DIR=${WXWIDGET_INCLUDE_DIR})
message(WXWIDGET_LIBRARY_DIR=${WXWIDGET_LIBRARY_DIR})
else()
message(wxWidgets_USE_FILE=${wxWidgets_USE_FILE})
message(wxWidgets_LIBRARIES=${wxWidgets_LIBRARIES})
endif()
message(=========== 3.JRTPLIB ==============)
message(JRTPLIB_INCLUDE_DIRS=${JRTPLIB_INCLUDE_DIRS})
message(JRTPLIB_LIBRARIES=${JRTPLIB_LIBRARIES})

message(====FFMPEG_INFO====)
message("--------------------")
message(AVCODEC_INCLUDE_DIR=${AVCODEC_INCLUDE_DIR})
message(AVCODEC_LIBRARY=${AVCODEC_LIBRARY})
message("--------------------")
message(AVFORMAT_INCLUDE_DIR=${AVFORMAT_INCLUDE_DIR})
message(AVFORMAT_LIBRARY=${AVFORMAT_LIBRARY})
message("--------------------")
message(AVUTIL_INCLUDE_DIR=${AVUTIL_INCLUDE_DIR})
message(AVUTIL_LIBRARY=${AVUTIL_LIBRARY})
message("--------------------")
message(AVDEVICE_INCLUDE_DIR=${AVDEVICE_INCLUDE_DIR})
message(AVDEVICE_LIBRARY=${AVDEVICE_LIBRARY})
message("--------------------")
message(AVFILTER_INCLUDE_DIR=${AVFILTER_INCLUDE_DIR})
message(AVFILTER_LIBRARY=${AVFILTER_LIBRARY})
message("--------------------")
message(SWRESAMPLE_INCLUDE_DIR=${SWRESAMPLE_INCLUDE_DIR})
message(SWRESAMPLE_LIBRARY=${SWRESAMPLE_LIBRARY})
message("--------------------")
message(SWSCALE_INCLUDE_DIR=${SWSCALE_INCLUDE_DIR})
message(SWSCALE_LIBRARY=${SWSCALE_LIBRARY})

message(====OPENAL_INFO====)
message(OPENAL32_LIBRARY=${OPENAL32_LIBRARY})
message(AL_INCLUDE_DIR=${AL_INCLUDE_DIR})

message(AEC3_LIB = ${ECHO_LIBRARY})


# 4.========================config配置========================


set(LOG_FILE_NAME "application.log")
set(SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE)
set(REMOTE_IP "47.93.119.6")
set(REMOTE_PORT 52010)
set(COTURN_USERNAME "sk8x")
set(COTURN_PASSWORD "sk86")

set(WS_PTCL "ws://")
set(SPINDLE_HOST "47.93.119.6")
set(SPINDLE_PORT 30404)
set(SERIA_HOST "123.56.108.66")
set(SERIA_PORT 30406)

set(NOT_USE_CONSOLE 0)
set(TA 100)
set(MAX_RETRANS_COUNT 5)

configure_file(
		"config.h.in"
		"${PROJECT_SOURCE_DIR}/include/config.h"
		@ONLY
)


# 5. ========================需要编译的文件列表========================

# !!! 每次添加文件后需要reload cmake !!!
add_compile_dir(src/ui)
add_compile_dir(src/media)
add_compile_dir(src/utils)
add_compile_dir(src/websocket)


# 6. ========================包含main函数的文件运行========================
add_execute(src/Boot.cpp ${PROJECT_NAME} false)
#add_execute(src/test/windowCapture.cpp ${PROJECT_NAME} false)
#add_execute(src/test/windowCapture2.cpp ${PROJECT_NAME} false)
#add_execute(src/test/again.cpp ${PROJECT_NAME} false)
#add_execute(src/test/winrt_capture.cpp ${PROJECT_NAME} false)
#add_execute(src/test/testVideoCapture.cpp ${PROJECT_NAME} false)
#add_execute(src/test/testupanddenoise.cpp testupanddenoise false)
#add_execute_test(src/test/recordmic.cpp recordmic)
#add_execute_test(src/test/Capture.cpp Capture)
#add_execute(src/test/testErase.cpp testErase)
#add_execute(src/test/testMap.cpp testMap)
#add_execute(src/test/testController.cpp testController)
#add_execute(src/test/vectorTest.cpp vectorTest)
#add_execute(src/test/connectTest.cpp connectTest)
#add_execute(src/test/IceRequestTest.cpp IceRequestTest)
#add_execute(src/test/main2.cpp main2)
#add_execute(src/test/test.cpp test)
#add_execute(src/test/test2.cpp test2)
#add_execute(src/test/msgTest.cpp msgTest)
#add_execute(src/test/jsonTest.cpp jsonTest)
#add_execute(src/test/timerTest.cpp timerTest)
#add_execute(src/test/testPair.cpp testPair)
#add_execute(src/test/openal.cpp openal)
#add_execute(src/test/rtplib.cpp rtplib)
#add_execute(src/test/rtpConnTest.cpp rtpConnTest)
#add_execute(src/test/MediaMixTest.cpp MediaMixTest false)

#include(CMakeCPack.cmake)
