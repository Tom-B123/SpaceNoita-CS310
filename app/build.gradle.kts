// Plugins to make gradle work for managing CPP and testing
plugins {
    `cpp-application`

    `cpp-unit-test`
}

application {
    source.from(file("src/main/cpp"))
    privateHeaders.from(file("src/main/headers"))
    privateHeaders.from(file("src/main/headers/include"))
    targetMachines.add(machines.windows.x86_64)
}

tasks.withType(CppCompile::class).configureEach {
    // OpenCL
    compilerArgs.add("-IC:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/include")
    compilerArgs.add("-IC:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/include")

    // Open GL
    compilerArgs.add("-DGLEW_STATIC")
    compilerArgs.add("-DGLFW_STATIC")

}


tasks.withType(LinkExecutable::class).configureEach {

    // Open CL
    linkerArgs.add("C:/Users/tomhb/University/OpenCL/OpenCL-SDK/install/lib/OpenCL.lib")
    // Open GL
    linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glew32s.lib")
    linkerArgs.add("C:/Users/tomhb/University/cs310/FallingSandIter1/app/src/main/headers/lib/glfw3.lib")
    linkerArgs.add("opengl32.lib")

    // Additional dependencies
    linkerArgs.add("user32.lib")
    linkerArgs.add("gdi32.lib")
    linkerArgs.add("shell32.lib")

    linkerArgs.add("msvcrt.lib")
}

// On gradle.bat run, build then run the exe
tasks.register<Exec>("run") {
    dependsOn(tasks.build)

    val exePath = "C:/Users/tomhb/University/cs310/FallingSandIter1/app/build/exe/main/debug/app.exe"
    commandLine("cmd", "/c", exePath)

}

// Make 'run' the default task when no arguments are provided
defaultTasks("build", "run")
