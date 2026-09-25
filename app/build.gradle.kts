/*
 * DISCLAIMER: Deepseek was used to help with setting up the libraries and handling cross platform settings
 */


import org.gradle.internal.os.OperatingSystem
// Plugins to make gradle work for managing CPP and testing
plugins {
    `cpp-application`

    `cpp-unit-test`
}

application {
    source.from(file("src/main/cpp"))
    privateHeaders.from(file("src/main/headers"))
    targetMachines.set(listOf(machines.linux.x86_64, machines.windows.x86_64))
}

tasks.withType(CppCompile::class).configureEach {
    // Common defines
    compilerArgs.add("-DGLEW_STATIC")
    compilerArgs.add("-DGLFW_STATIC")
    
    if (OperatingSystem.current().isWindows) {
        // Windows-specific include paths
        compilerArgs.add("-IC:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/include")
        compilerArgs.add("-IC:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/include")
    } else {
        // Linux: headers are in standard system locations
        // Usually no extra -I flags are needed for GLEW/GLFW
        // OpenCL headers also in /usr/include on Arch
    }
}
// tasks.withType(CppCompile::class).configureEach {
//     // OpenCL
//     compilerArgs.add("-IC:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/include")
//     compilerArgs.add("-IC:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/include")
//
//     // Open GL
//     compilerArgs.add("-DGLEW_STATIC")
//     compilerArgs.add("-DGLFW_STATIC")
//
// }


// tasks.withType(LinkExecutable::class).configureEach {
//
//     // Open CL
//     linkerArgs.add("C:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/lib/OpenCL.lib")
//     // Open GL
//     linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glew32s.lib")
//     linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glfw3.lib")
//     linkerArgs.add("opengl32.lib")
//
//     // Additional dependencies
//     linkerArgs.add("user32.lib")
//     linkerArgs.add("gdi32.lib")
//     linkerArgs.add("shell32.lib")
//
//     linkerArgs.add("msvcrt.lib")
// }

tasks.withType(LinkExecutable::class).configureEach {
    if (OperatingSystem.current().isWindows) {
        // Your existing Windows linker args
        linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glew32s.lib")
        linkerArgs.add("C:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/lib/OpenCL.lib")
        linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glfw3.lib")
        linkerArgs.add("user32.lib")
        linkerArgs.add("gdi32.lib")
        linkerArgs.add("shell32.lib")
        linkerArgs.add("ole32.lib")
        linkerArgs.add("winmm.lib")
        linkerArgs.add("ws2_32.lib")
        linkerArgs.add("opengl32.lib")
        linkerArgs.add("advapi32.lib")
    } else {
        // Linux: link against system-installed libraries
        linkerArgs.add("-lGLEW")      // GLEW (libGLEW.so)
        linkerArgs.add("-lglfw")      // GLFW (libglfw.so) — note: NOT -lglfw3 [citation:7]
        linkerArgs.add("-lOpenCL")    // OpenCL ICD loader
        linkerArgs.add("-lGL")        // OpenGL
        linkerArgs.add("-ldl")        // For dlopen, needed by some GL loaders
        linkerArgs.add("-lpthread")   // For threading
        linkerArgs.add("-lm")         // Math library
    }
}

// On gradle.bat run, build then run the exe
// tasks.register<Exec>("run") {
//     dependsOn(tasks.build)
//
//     val exePath = "C:/Users/tomhb/University/cs310/FallingSandIter1/app/build/exe/main/debug/app.exe"
//     commandLine("cmd", "/c", exePath)
//
// }

tasks.register<Exec>("run") {
    dependsOn(tasks.build)
    
    val basePath = "C:/Users/tomhb/University/cs310/FallingSandIter1/app/build/exe/main"
    
    if (OperatingSystem.current().isWindows) {
        val exePath = "$basePath/debug/app.exe"
        commandLine("cmd", "/c", exePath)
    } else {
        val exePath = "$basePath/debug/app"
        commandLine(exePath)
    }
}

// Make 'run' the default task when no arguments are provided
defaultTasks("build", "run")
