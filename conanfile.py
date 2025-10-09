from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain


class ExampleRecipe(ConanFile):
    name = "thread-pool-sorter"
    version = "0.0.0"
    description = "Linux 环境高级编程的课程作业。"
    languages = "C++"
    author = "DavidingPlus"
    homepage = "https://github.com/DavidingPlus/thread-pool-sorter"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    options = {
        "with_gtest": [True, False]
    }
    default_options = {
        "with_gtest": True
    }

    def requirements(self):
        if self.options.with_gtest:
            self.requires("gtest/1.12.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.presets_prefix = "thread-pool-sorter"
        tc.cache_variables["PACKAGE_VERSION"] = self.version
        tc.cache_variables["with_gtest"] = self.options.with_gtest
        tc.generate()
