# 🎵 **VLAK – Lossless Audio Codec**

## 🕰️ History & Motivation

This project is a digital relic from the early days of modern computing, originally developed by a group of [students](#Authors) during their Computer Science studies at the **Warsaw University of Technology** in the **2005/2006 academic year** (05Z semester).

Motivation for revisiting this project is two-fold:

1. 🗃️ **Preservation** – To safeguard the source code and intellectual property we created, ensuring it remains accessible to future generations.  
2. 🤖 **Modernization** – To experiment with AI-powered tools in order to restore and refactor this legacy codebase.

## 📌 Goal

The project aimed to create a **lossless audio codec** (bit-perfect compression without quality loss) as an alternative to popular lossy codecs such as MP3 and OGG.  

Main applications include:
- archiving CD-quality music,
- professional audio processing,
- audiophile use,
- bootlegs and live recordings,
- portable music players.

## 🧰 Inspiration and References
The authors analyzed several existing codecs:
- _FLAC_ – open standard and the main reference model for VLAK’s architecture,
- _Shorten_ – simple FIR and LPC predictor,
- _Monkey's Audio_ – higher efficiency using neural network prediction,
- _La_ – very high compression at the cost of speed,
- _Windows Media Audio_,
- _Apple Lossless Audio Codec_ (ALAC).

## 🏗️ Building

The project uses CMake as its build system. To build the project:

```bash
mkdir build
cd build
cmake ..
make
```

### ⚙️ Requirements
- C++11 compatible compiler
- CMake 3.10 or higher

## 🚀 Usage

```
VLAK - flac inspired audio codec
(c) Piotr Malinowski, Wojtek Wasiak, Krzysiek Jamroz, Mateusz Grzegorzek (05z)
==============================================================================
Encoding: ./vlak [encoding-options] INPUTWAVFILE [OUTPUTVLAKFILE]
Decoding: ./vlak -d [decoding-options] VLAKFILE [OUTPUTWAVFILE]
Analyzing: ./vlak -a [analysis-options] VLAKFILE
general options:
-d, --decode
-h, --help
-a, --analyze
encoding options:
-p, --predictor=#
-b, --blocksize=#
-m, --manipulator=#
-l, --lpcorder=#,#
--fft-qfactor=#
--fft-minrelampl=#
analysis options: (mutually exclusive)
--residual-distrib
--residual-signal
--original-signal
--manipulated-signal
--predicted-signal
--predictor-order
--info
--compression-ratio
predictor types:
0 : Simple
1 : LPC
2 : FIR
3 : Wavelet
4 : FFT
manipulator types:
0 : CSimpleChannelManipulator
1 : CMidSideChannelManipulator
2 : CAdaptiveChannelManipulator
defaults:
predictor type   : LPC
manipulator type : CAdaptiveChannelManipulator
blocksize        : 1024
lpcorder         : 1,12
```

### 📋 Examples

#### 🔹 Basic Compression

To compress a WAV file:

```bash
./vlak input.wav
```

This will create `input.vlak` as the output file.

#### 🔹 Basic Decompression

To decompress a VLAK file:

```bash
./vlak -d input.vlak
```

This will create `input.wav` as the output file.

## 🛠️ Tools
- `vlak` – Main compression/decompression tool
- `wavdiff` – Utility to compare WAV files

The project includes several tools:

- `vlak` - Main compression/decompression tool
- `wavdiff` - Utility to compare WAV files

## 🧠 Codec Design
- **Block** – a fragment of the WAV signal (N samples per channel).
- **Predictor** – transforms the block into a form that is easier to compress.
- **Frame** – contains compressed data and the residual signal.

### 🧮 Prediction Methods
1. **Finite Impulse Response (FIR)** – fast, simple algorithm; good compression with larger blocks.  
2. **Linear Predictive Coding (LPC)** – linear prediction with Levinson-Durbin algorithm; adaptive order selection.  
3. **Fast Fourier Transform (FFT)** – original frequency-domain prediction method with spectrum quantization.  
4. **Wavelet** – wavelet transform (block sizes as powers of two).  
5. **Simple** – basic no compression / fallback mode.

### 🔸 Entropy Coding
- Uses a **Rice coder** (a variant of Golomb coding), optimal for data concentrated around zero.  

### 🧭 Channel Manipulation
- **Mid-Side transform** improves compression by about 1% with minimal processing cost.
- Particularly effective for “fake stereo” recordings.

## 🧪 Tests and Results
- Tests were performed on recordings of different music genres (reggae, rock, metal, classical, etc.).
- Measured:
  - **CR (Compression Ratio)** = original size / compressed size,
  - **CP (Compression Percentage)** = saved space percentage.
- Best performance came from **LPC** and **Wavelet** predictors with well-chosen block sizes.
- **FFT** showed slow compression but good potential with proper tuning.
- Wavelet efficiency depends on block lengths that are powers of two.

## ⏳ Performance
- **FIR** – constant compression time, independent of block length; very fast.
- **LPC** – time increases with block size and predictor order.
- **Wavelet** – logarithmic time increase with block length.

## 📈 Comparison with Other Codecs
- VLAK achieved compression efficiency comparable to FLAC and Shorten.
- It was slower than FLAC but offered greater flexibility in prediction algorithms.
- General-purpose codecs such as gzip and bzip2 performed significantly worse on audio data.

## 🛠️ Potential Improvements
- adaptive parameter and quantization selection,
- more advanced entropy coding (e.g., Huffman),
- better channel decorrelation,
- optimized FFT and Wavelet implementations.

## 📜 Final Summary
The VLAK project proves that a **custom-built lossless audio codec** can match popular open-source solutions in terms of compression ratio.  
Although not the fastest, its architecture is flexible and easy to extend.  
The **LPC** and **Wavelet** predictors were the most promising, and Mid-Side transformation provided simple but effective compression gains.

## 👨‍💻 Authors
- Krzysztof Jamróz
- Mateusz Grzegorzek
- Piotr Malinowski
- Wojciech Wasiak

## 📝 License
[License information should be added here]

## 📚 Documentation
Additional documentation can be found in the `docs/` directory:
- [`koda_sprawozdanie.pdf`](docs/koda_sprawozdanie.pdf) - original report in polish 🇵🇱