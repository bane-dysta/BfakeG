--以下内容由Claude4生成，请忽略彩红屁，看看使用部分就好--

# BDFakeG

**A utility to convert BDF quantum chemistry output files to Gaussian-compatible format**

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/bane-dysta/BfakeG)
[![Language](https://img.shields.io/badge/language-C%2B%2B11-orange.svg)](https://isocpp.org/)


## 📖 简介 | Overview

BDFakeG 是一个将 BDF (Beijing Density Functional) 量子化学计算输出文件转换为 Gaussian 兼容格式的工具。它允许研究人员使用基于 Gaussian 的分析工具和可视化软件来处理 BDF 的计算结果。

BDFakeG is a utility that converts BDF (Beijing Density Functional) quantum chemistry output files into Gaussian-compatible log format, enabling researchers to use Gaussian-based analysis tools and visualization software with BDF calculation results.

## ✨ 功能特性 | Features

- 🔄 **完整数据转换** | Complete data conversion
  - 几何优化轨迹 | Geometry optimization trajectories
  - 频率分析结果 | Vibrational frequency analysis
  - 热力学性质 | Thermodynamic properties
  - 收敛信息 | Convergence information

- 📊 **支持的计算类型** | Supported calculation types
  - 单点能计算 | Single point calculations
  - 几何优化 | Geometry optimization
  - 频率计算 | Frequency calculations
  - 热化学分析 | Thermochemical analysis

- 🎯 **高保真输出** | High fidelity output
  - 保持数据完整性 | Preserves data integrity
  - Gaussian 格式兼容 | Gaussian format compatibility
  - 对称性信息转换 | Symmetry information conversion

## 🚀 快速开始 | Quick Start

### 编译 | Compilation

```bash
# 使用 g++ 编译
g++ -std=c++11 -O2 BDFakeG.cpp -o BDFakeG

# 或使用 clang++
clang++ -std=c++11 -O2 BDFakeG.cpp -o BDFakeG
```

### 使用方法 | Usage

```bash
# 交互式模式
./BDFakeG

# 直接指定输入文件
./BDFakeG calculation.out

# 调试模式（推荐用于排查问题）
./BDFakeG calculation.out --debug
```

### 输入输出 | Input/Output

- **输入** | Input: BDF 输出文件 (如 `calculation.out`)
- **输出** | Output: Gaussian 兼容的 log 文件 (如 `calculation_fake.log`)

## 📋 使用示例 | Examples

### 基本转换 | Basic Conversion
```bash
$ ./BDFakeG h2_optimization.out
BDFakeG: Generate fake Gaussian output from BDF file
Author: Bane-Dysta
Processing file: h2_optimization.out
Found geometry optimization
Total optimization steps: 4
Total frequencies parsed: 1

Fake Gaussian output written to: h2_optimization_fake.log
```

### 调试模式 | Debug Mode
```bash
$ ./BDFakeG h2_optimization.out --debug
# 输出详细的解析信息，用于排查问题
```

## 📄 支持的数据格式 | Supported Data

| 数据类型 | BDF输入 | Gaussian输出 | 状态 |
|---------|---------|-------------|------|
| 单点能 \| Single Point | ✅ | ✅ | 完全支持 |
| 几何优化 \| Optimization | ✅ | ✅ | 完全支持 |
| 频率分析 \| Frequencies | ✅ | ✅ | 完全支持 |
| 热力学数据 \| Thermodynamics | ✅ | ✅ | 完全支持 |
| 收敛信息 \| Convergence | ✅ | ✅ | v1.1.0 新增 |
| IR 强度 \| IR Intensities | ✅ | ✅ | 完全支持 |

## 🔧 系统要求 | Requirements

- **编译器** | Compiler: C++11 兼容的编译器 (GCC 4.8+, Clang 3.3+, MSVC 2015+)
- **操作系统** | OS: Windows, Linux, macOS
- **依赖** | Dependencies: 仅标准 C++ 库

## 🐛 故障排除 | Troubleshooting

### 常见问题 | Common Issues

**Q: 转换后的文件缺少某些数据？**
**Q: Converted file missing some data?**

A: 使用 `--debug` 模式运行，查看详细的解析信息。确保 BDF 输出文件完整且未被截断。

A: Run with `--debug` mode to see detailed parsing information. Ensure your BDF output file is complete and not truncated.

**Q: 收敛信息显示为零？**
**Q: Convergence information shows zeros?**

A: 这个问题在 v1.1.0 中已修复。请更新到最新版本。

A: This issue has been fixed in v1.1.0. Please update to the latest version.

**Q: 编译失败？**
**Q: Compilation failed?**

A: 确保使用 C++11 标准：`g++ -std=c++11 BDFakeG.cpp -o BDFakeG`

A: Make sure to use C++11 standard: `g++ -std=c++11 BDFakeG.cpp -o BDFakeG`

## 📈 版本历史 | Version History

### v1.1.0 (最新 | Latest)
- 🐛 修复几何优化最后一步收敛数据归零问题
- ✨ 添加频率分析梯度收敛信息输出
- 🔧 增强解析算法的鲁棒性

### v1.0.0
- 🎉 初始版本发布
- ✅ 基本的 BDF 到 Gaussian 格式转换
- ✅ 支持几何优化、频率计算和热力学数据

## 🤝 贡献 | Contributing

欢迎提交 Issue 和 Pull Request！

Welcome to submit Issues and Pull Requests!

### 报告问题 | Reporting Issues
- 使用 `--debug` 模式生成详细日志
- 提供输入的 BDF 文件和调试输出
- 说明你的 BDF 版本和计算类型

### 开发 | Development
- Fork 本项目
- 创建功能分支
- 提交 Pull Request

## 👨‍💻 作者 | Author

[**Bane-Dysta**](https://bane-dysta.top/)

## 🙏 致谢 | Acknowledgments

感谢所有使用和改进 BDFakeG 的用户和贡献者。

Thanks to all users and contributors who use and improve BDFakeG.

---

*如果这个工具对你的研究有帮助，请考虑给项目加个星标 ⭐*

*If this tool helps your research, please consider giving the project a star ⭐*