set -e
FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

pacman --noconfirm -Sy
pacman --noconfirm -S msys/make
pacman --noconfirm -S diffutils
pacman --noconfirm -S pkg-config

cd $FullScriptPath/../FFmpeg

export PKG_CONFIG_PATH="$FullExecPath/../local/lib/pkgconfig:$PKG_CONFIG_PATH"

./configure --prefix=$FullScriptPath/../local \
--toolchain=msvc \
--extra-cflags="-DCONFIG_SAFE_BITSTREAM_READER=1" \
--extra-cxxflags="-DCONFIG_SAFE_BITSTREAM_READER=1" \
--disable-programs \
--disable-doc \
--disable-network \
--disable-everything \
--enable-protocol=file \
--enable-libvpx \
--enable-decoder=aac \
--enable-decoder=aac_fixed \
--enable-decoder=aac_latm \
--enable-decoder=aasc \
--enable-decoder=ac3 \
--enable-decoder=alac \
--enable-decoder=eac3 \
--enable-decoder=flac \
--enable-decoder=mp1 \
--enable-decoder=mp1float \
--enable-decoder=mp2 \
--enable-decoder=mp2float \
--enable-decoder=mp3 \
--enable-decoder=mp3adu \
--enable-decoder=mp3adufloat \
--enable-decoder=mp3float \
--enable-decoder=mp3on4 \
--enable-decoder=mp3on4float \
--enable-decoder=pcm_alaw \
--enable-decoder=pcm_f32be \
--enable-decoder=pcm_f32le \
--enable-decoder=pcm_f64be \
--enable-decoder=pcm_f64le \
--enable-decoder=pcm_lxf \
--enable-decoder=pcm_mulaw \
--enable-decoder=pcm_s16be \
--enable-decoder=pcm_s16be_planar \
--enable-decoder=pcm_s16le \
--enable-decoder=pcm_s16le_planar \
--enable-decoder=pcm_s24be \
--enable-decoder=pcm_s24daud \
--enable-decoder=pcm_s24le \
--enable-decoder=pcm_s24le_planar \
--enable-decoder=pcm_s32be \
--enable-decoder=pcm_s32le \
--enable-decoder=pcm_s32le_planar \
--enable-decoder=pcm_s64be \
--enable-decoder=pcm_s64le \
--enable-decoder=pcm_s8 \
--enable-decoder=pcm_s8_planar \
--enable-decoder=pcm_u16be \
--enable-decoder=pcm_u16le \
--enable-decoder=pcm_u24be \
--enable-decoder=pcm_u24le \
--enable-decoder=pcm_u32be \
--enable-decoder=pcm_u32le \
--enable-decoder=pcm_u8 \
--enable-decoder=vorbis \
--enable-decoder=wavpack \
--enable-decoder=wmalossless \
--enable-decoder=wmapro \
--enable-decoder=wmav1 \
--enable-decoder=wmav2 \
--enable-decoder=wmavoice \
--enable-parser=aac \
--enable-parser=aac_latm \
--enable-parser=flac \
--enable-parser=mpegaudio \
--enable-parser=vorbis \
--enable-demuxer=aac \
--enable-demuxer=flac \
--enable-demuxer=mp3 \
--enable-demuxer=ogg \
--enable-demuxer=wav \
--enable-muxer=ogg

make -j8
make -j8 install
