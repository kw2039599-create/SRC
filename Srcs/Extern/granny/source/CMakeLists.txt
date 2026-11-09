cmake_minimum_required(VERSION 3.10)

project(granny2_static
    DESCRIPTION "Granny2 Static Library for Windows / Android / iOS"
    VERSION "2.9.12.0"
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILDING_GRANNY "Build Granny Library For Dll Files" OFF)
option(BUILDING_GRANNY_STATIC "Build Granny Static Library" ON)
option(RAD_NO_LOWERCASE_TYPES "Disable Lowercase Type Definitions" ON)
option(_HAS_ITERATOR_DEBUGGING "Disable Iterator Debugging" OFF)

add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)

add_library(${PROJECT_NAME} STATIC
    arithbit.c
    binktc.c
    encode.c
    radarith.c
    radlz.c
    rrCore.c
    varbits.c
    wavelet.c

    contain.inl
    granny_contain.inl
    granny_generic_deformer_wrapper.inl
    granny_generic_deformers.inl
    radarith_exact.inl
    sort.inl

    rrBase.cpp
    rrMath.cpp

    granny_aggr_alloc.cpp
    granny_animation.cpp
    granny_animation_binding.cpp
    granny_art_tool_info.cpp
    granny_assert.cpp
    granny_back_compat.cpp
    granny_basis_conversion.cpp
    granny_binary_search.cpp
    granny_bink.cpp
    granny_bink0_compression.cpp
    granny_bink1_compression.cpp
    granny_bone_operations.cpp
    granny_bspline.cpp
    granny_bspline_buffers.cpp
    granny_bspline_solver.cpp
    granny_bspline_solver_core.cpp
    granny_camera.cpp
    granny_catenating_reader.cpp
    granny_compress_curve.cpp
    granny_compression_tools.cpp
    granny_constants.cpp
    granny_control.cpp
    granny_controlled_animation.cpp
    granny_controlled_pose.cpp
    granny_crc.cpp
    granny_curve.cpp
    granny_curve_fast.cpp
    granny_data_type_io.cpp
    granny_data_type_conversion.cpp
    granny_data_type_definition.cpp
    granny_deformers.cpp
    granny_degree_of_freedom.cpp
    granny_exporter_info.cpp

    granny_file.cpp
    granny_file_builder.cpp
    granny_file_compressor.cpp
    granny_file_format.cpp
    granny_file_info.cpp
    granny_file_operations.cpp
    granny_file_reader.cpp
    granny_file_writer.cpp
    granny_find_knot.cpp
    granny_fixed_allocator.cpp
    granny_float16.cpp
    granny_floats.cpp
    granny_generic_deformers.cpp
    granny_ik.cpp
    granny_image_operations.cpp
    granny_intersection.cpp
    granny_local_pose.cpp
    granny_log.cpp
    granny_lookat.cpp
    granny_material.cpp
    granny_math.cpp
    granny_matrix_operations.cpp
    granny_member_iterator.cpp
    granny_memory.cpp
    granny_memory_arena.cpp
    granny_memory_file_reader.cpp
    granny_memory_file_writer.cpp
    granny_memory_ops.cpp
    granny_mesh.cpp
    granny_mesh_binding.cpp
    granny_mesh_builder.cpp
    granny_mesh_deformer.cpp
    granny_mirror_specification.cpp
    granny_mod_support.cpp
    granny_model.cpp
    granny_model_control_binding.cpp
    granny_model_instance.cpp

    granny_oodle0_compression.cpp
    granny_oodle1_compression.cpp
    granny_periodic_loop.cpp
    granny_pixel_layout.cpp
    granny_platform_convert.cpp
    granny_pointer_hash.cpp
    granny_pose_cache.cpp
    granny_quaternion_scaleoffset.cpp
    granny_retargeter.cpp
    granny_s3tc.cpp
    granny_sieve.cpp
    granny_skeleton.cpp
    granny_skeleton_builder.cpp

    granny_stack_allocator.cpp
    granny_string.cpp
    granny_string_comparison_callback.cpp
    granny_string_database.cpp
    granny_string_formatting.cpp
    granny_string_table.cpp
    granny_tangent_frame.cpp
    granny_telemetry.cpp
    granny_test_types.cpp
    granny_texture.cpp
    granny_texture_builder.cpp
    granny_thread_safety.cpp
    granny_track_group.cpp
    granny_track_group_builder.cpp
    granny_track_group_sampler.cpp
    granny_track_mask.cpp
    granny_track_sampler.cpp
    granny_transform.cpp
    granny_tri_topology.cpp
    granny_type_listing.cpp
    granny_units.cpp
    granny_variant_builder.cpp
    granny_version_checking.cpp
    granny_vertex_data.cpp
    granny_world_pose.cpp
)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        winxx/winxx_granny_assert.cpp
        winxx/winxx_granny_dll.cpp
        winxx/winxx_granny_file_operations.cpp
        winxx/winxx_granny_file_reader.cpp
        winxx/winxx_granny_file_writer.cpp
        winxx/winxx_granny_memory.cpp
        winxx/winxx_granny_startup.cpp
        winxx/winxx_granny_system_clock.cpp
        winxx/winxx_granny_threads.cpp
        winxx/winxx_granny_windows.cpp
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_sources(${PROJECT_NAME} PRIVATE
            winxx/win32_rrAtomics.cpp
            winxx/win32/granny.cpp
            winxx/win32/x86_granny_accelerated_deformers.cpp
            winxx/win32/x86_granny_bone_operations.cpp
            winxx/win32/x86_granny_cpu_queries.cpp
            winxx/win32/x86_granny_matrix_operations.cpp
        )
        target_include_directories(${PROJECT_NAME} PUBLIC
            winxx/
            winxx/win32/
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
    else()
        target_sources(${PROJECT_NAME} PRIVATE
            winxx/win64/ansi_granny_accelerated_deformers.cpp
            winxx/win64/ansi_granny_bone_operations.cpp
            winxx/win64/ansi_granny_matrix_operations.cpp
            winxx/win64/granny.cpp
            winxx/win64/win64_rrAtomics.cpp
        )
        target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            winxx/win64/
            winxx/
        )
    endif()
else()
    # Android / iOS / Linux / macOS portable ANSI + POSIX build
    target_sources(${PROJECT_NAME} PRIVATE
        posix_granny_platform.cpp
        winxx/win64/ansi_granny_accelerated_deformers.cpp
        winxx/win64/ansi_granny_bone_operations.cpp
        winxx/win64/ansi_granny_matrix_operations.cpp
        winxx/win64/granny.cpp
    )

    if(ANDROID OR APPLE)
        # Exclude Windows winxx/win64 headers on Android to prevent CONTAINER_INTPTR size mismatches
        target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ../include/
        )
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -fno-strict-return -fno-strict-aliasing)
    else()
        target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ../include/
            winxx/win64/
        )
    endif()
endif()

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    BUILDING_GRANNY_STATIC=1
    PRIVATE
    RAD_NO_LOWERCASE_TYPES=0
    _HAS_ITERATOR_DEBUGGING=0
)