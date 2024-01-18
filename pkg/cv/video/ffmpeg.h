#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_H

namespace kingfisher {
namespace cv {

#if 0
// 数据结构，用于存储键值对,例如视频解码参数、音频参数、字幕参数等等
// AVDictionary
//
//struct AVDictionary {
    int count;
    AVDictionaryEntry *elems;
};
typedef struct AVDictionaryEntry {
    char *key;
    char *value;
} AVDictionaryEntry;

/*
  https://sourcegraph.com/github.com/FFmpeg/FFmpeg@ae14d9c06bcddc5d4c14de02e049f489ddbf73a4/-/blob/libavformat/avformat.h?L1201#:~:text=typedef%20struct%20AVFormatContext%20%7B

AVFormatContext
用于处理媒体文件的输入和输出。它包含了与媒体文件（例如视频、音频或字幕文件）相关的信息，如文件格式、流信息、元数据等。在FFmpeg中，AVFormatContext被用作一个中心结构，用于管理与容器格式相关的操作，如解复用（demuxing，从容器中提取音频、视频和字幕流）和复用（muxing，将音频、视频和字幕流封装到容器中）

/**
 * Format I/O context.
 * New fields can be added to the end with minor version bumps.
 * Removal, reordering and changes to existing fields require a major
 * version bump.
 * sizeof(AVFormatContext) must not be used outside libav*, use
 * avformat_alloc_context() to create an AVFormatContext.
 *
 * Fields can be accessed through AVOptions (av_opt*),
 * the name string used matches the associated command line parameter name and
 * can be found in libavformat/options_table.h.
 * The AVOption/command line parameter names differ in some cases from the C
 * structure field names for historic reasons or brevity.
 */

typedef struct AVFormatContext {
/**
 * A class for logging and @ref avoptions. Set by avformat_alloc_context().
 * Exports (de)muxer private options if they exist.
 */
const AVClass *av_class;

/**
 * The input container format.
 *
 * Demuxing only, set by avformat_open_input().
 */
//输入格式
const struct AVInputFormat *iformat;

/**
 * The output container format.
 *
 * Muxing only, must be set by the caller before avformat_write_header().
 */
//输输出格式
const struct AVOutputFormat *oformat;

/**
 * Format private data. This is an AVOptions-enabled struct
 * if and only if iformat/oformat.priv_class is not NULL.
 *
 * - muxing: set by avformat_write_header()
 * - demuxing: set by avformat_open_input()
 */
void *priv_data;

/**
 * I/O context.
 *
 * - demuxing: either set by the user before avformat_open_input() (then
 *             the user must close it manually) or set by avformat_open_input().
 * - muxing: set by the user before avformat_write_header(). The caller must
 *           take care of closing / freeing the IO context.
 *
 * Do NOT set this field if AVFMT_NOFILE flag is set in
 * iformat/oformat.flags. In such a case, the (de)muxer will handle
 * I/O in some other way and this field will be NULL.
 */
AVIOContext *pb;

/* stream info */
/**
 * Flags signalling stream properties. A combination of AVFMTCTX_*.
 * Set by libavformat.
 */
int ctx_flags;

/**
 * Number of elements in AVFormatContext.streams.
 *
 * Set by avformat_new_stream(), must not be modified by any other code.
 */
//表示一个多媒体文件中包含的音频、视频和其他数据流的总数量。例如，一个包含音频和视频的多媒体文件的 nb_streams 值为 2
unsigned int nb_streams;
/**
 * A list of all streams in the file. New streams are created with
 * avformat_new_stream().
 *
 * - demuxing: streams are created by libavformat in avformat_open_input().
 *             If AVFMTCTX_NOHEADER is set in ctx_flags, then new streams may
 * also appear in av_read_frame().
 * - muxing: streams are created by the user before avformat_write_header().
 *
 * Freed by libavformat in avformat_free_context().
 */
AVStream **streams;

/**
 * input or output URL. Unlike the old filename field, this field has no
 * length restriction.
 *
 * - demuxing: set by avformat_open_input(), initialized to an empty
 *             string if url parameter was NULL in avformat_open_input().
 * - muxing: may be set by the caller before calling avformat_write_header()
 *           (or avformat_init_output() if that is called first) to a string
 *           which is freeable by av_free(). Set to an empty string if it
 *           was NULL in avformat_init_output().
 *
 * Freed by libavformat in avformat_free_context().
 */
char *url;

/**
 * Position of the first frame of the component, in
 * AV_TIME_BASE fractional seconds. NEVER set this value directly:
 * It is deduced from the AVStream values.
 *
 * Demuxing only, set by libavformat.
 */
int64_t start_time;

/**
 * Duration of the stream, in AV_TIME_BASE fractional
 * seconds. Only set this value if you know none of the individual stream
 * durations and also do not set any of them. This is deduced from the
 * AVStream values if not set.
 *
 * Demuxing only, set by libavformat.
 */
int64_t duration;

/**
 * Total stream bitrate in bit/s, 0 if not
 * available. Never set it directly if the file_size and the
 * duration are known as FFmpeg can compute it automatically.
 */
int64_t bit_rate;

unsigned int packet_size;
int max_delay;

/**
 * Flags modifying the (de)muxer behaviour. A combination of AVFMT_FLAG_*.
 * Set by the user before avformat_open_input() / avformat_write_header().
 */
int flags;
#define AVFMT_FLAG_GENPTS \
  0x0001  ///< Generate missing pts even if it requires parsing future frames.
#define AVFMT_FLAG_IGNIDX 0x0002  ///< Ignore index.
#define AVFMT_FLAG_NONBLOCK \
  0x0004  ///< Do not block when reading packets from input.
#define AVFMT_FLAG_IGNDTS \
  0x0008  ///< Ignore DTS on frames that contain both DTS & PTS
#define AVFMT_FLAG_NOFILLIN \
  0x0010  ///< Do not infer any values from other values, just return what is
          ///< stored in the container
#define AVFMT_FLAG_NOPARSE \
  0x0020  ///< Do not use AVParsers, you also must set AVFMT_FLAG_NOFILLIN as
          ///< the fillin code works on frames and no parsing -> no frames. Also
          ///< seeking to frames can not work if parsing to find frame
          ///< boundaries has been disabled
#define AVFMT_FLAG_NOBUFFER 0x0040  ///< Do not buffer frames when possible
#define AVFMT_FLAG_CUSTOM_IO \
  0x0080  ///< The caller has supplied a custom AVIOContext, don't avio_close()
          ///< it.
#define AVFMT_FLAG_DISCARD_CORRUPT 0x0100  ///< Discard frames marked corrupted
#define AVFMT_FLAG_FLUSH_PACKETS \
  0x0200  ///< Flush the AVIOContext every packet.
/**
 * When muxing, try to avoid writing any random/volatile data to the output.
 * This includes any random IDs, real-time timestamps/dates, muxer version, etc.
 *
 * This flag is mainly intended for testing.
 */
#define AVFMT_FLAG_BITEXACT 0x0400
#define AVFMT_FLAG_SORT_DTS \
  0x10000  ///< try to interleave outputted packets by dts (using this flag can
           ///< slow demuxing down)
#if FF_API_LAVF_PRIV_OPT
#define AVFMT_FLAG_PRIV_OPT \
  0x20000  ///< Enable use of private options by delaying codec open
           ///< (deprecated, does nothing)
#endif
#define AVFMT_FLAG_FAST_SEEK \
  0x80000  ///< Enable fast, but inaccurate seeks for some formats
#define AVFMT_FLAG_SHORTEST \
  0x100000  ///< Stop muxing when the shortest stream stops.
#define AVFMT_FLAG_AUTO_BSF \
  0x200000  ///< Add bitstream filters as requested by the muxer

/**
 * Maximum number of bytes read from input in order to determine stream
 * properties. Used when reading the global header and in
 * avformat_find_stream_info().
 *
 * Demuxing only, set by the caller before avformat_open_input().
 *
 * @note this is \e not  used for determining the \ref AVInputFormat
 *       "input format"
 * @sa format_probesize
 */
int64_t probesize;

/**
 * Maximum duration (in AV_TIME_BASE units) of the data read
 * from input in avformat_find_stream_info().
 * Demuxing only, set by the caller before avformat_find_stream_info().
 * Can be set to 0 to let avformat choose using a heuristic.
 */
int64_t max_analyze_duration;

const uint8_t *key;
int keylen;

unsigned int nb_programs;
AVProgram **programs;

/**
 * Forced video codec_id.
 * Demuxing: Set by user.
 */
enum AVCodecID video_codec_id;

/**
 * Forced audio codec_id.
 * Demuxing: Set by user.
 */
enum AVCodecID audio_codec_id;

/**
 * Forced subtitle codec_id.
 * Demuxing: Set by user.
 */
enum AVCodecID subtitle_codec_id;

/**
 * Maximum amount of memory in bytes to use for the index of each stream.
 * If the index exceeds this size, entries will be discarded as
 * needed to maintain a smaller size. This can lead to slower or less
 * accurate seeking (depends on demuxer).
 * Demuxers for which a full in-memory index is mandatory will ignore
 * this.
 * - muxing: unused
 * - demuxing: set by user
 */
unsigned int max_index_size;

/**
 * Maximum amount of memory in bytes to use for buffering frames
 * obtained from realtime capture devices.
 */
unsigned int max_picture_buffer;

/**
 * Number of chapters in AVChapter array.
 * When muxing, chapters are normally written in the file header,
 * so nb_chapters should normally be initialized before write_header
 * is called. Some muxers (e.g. mov and mkv) can also write chapters
 * in the trailer.  To write chapters in the trailer, nb_chapters
 * must be zero when write_header is called and non-zero when
 * write_trailer is called.
 * - muxing: set by user
 * - demuxing: set by libavformat
 */
unsigned int nb_chapters;
AVChapter **chapters;

/**
 * Metadata that applies to the whole file.
 *
 * - demuxing: set by libavformat in avformat_open_input()
 * - muxing: may be set by the caller before avformat_write_header()
 *
 * Freed by libavformat in avformat_free_context().
 */
//存储了媒体文件的元数据, 如标题、作者等
AVDictionary *metadata;

/**
 * Start time of the stream in real world time, in microseconds
 * since the Unix epoch (00:00 1st January 1970). That is, pts=0 in the
 * stream was captured at this real world time.
 * - muxing: Set by the caller before avformat_write_header(). If set to
 *           either 0 or AV_NOPTS_VALUE, then the current wall-time will
 *           be used.
 * - demuxing: Set by libavformat. AV_NOPTS_VALUE if unknown. Note that
 *             the value may become known after some number of frames
 *             have been received.
 */
int64_t start_time_realtime;

/**
 * The number of frames used for determining the framerate in
 * avformat_find_stream_info().
 * Demuxing only, set by the caller before avformat_find_stream_info().
 */
int fps_probe_size;

/**
 * Error recognition; higher values will detect more errors but may
 * misdetect some more or less valid parts as errors.
 * Demuxing only, set by the caller before avformat_open_input().
 */
int error_recognition;

/**
 * Custom interrupt callbacks for the I/O layer.
 *
 * demuxing: set by the user before avformat_open_input().
 * muxing: set by the user before avformat_write_header()
 * (mainly useful for AVFMT_NOFILE formats). The callback
 * should also be passed to avio_open2() if it's used to
 * open the file.
 */
AVIOInterruptCB interrupt_callback;

/**
 * Flags to enable debugging.
 */
int debug;
#define FF_FDEBUG_TS 0x0001

/**
 * Maximum buffering duration for interleaving.
 *
 * To ensure all the streams are interleaved correctly,
 * av_interleaved_write_frame() will wait until it has at least one packet
 * for each stream before actually writing any packets to the output file.
 * When some streams are "sparse" (i.e. there are large gaps between
 * successive packets), this can result in excessive buffering.
 *
 * This field specifies the maximum difference between the timestamps of the
 * first and the last packet in the muxing queue, above which libavformat
 * will output a packet regardless of whether it has queued a packet for all
 * the streams.
 *
 * Muxing only, set by the caller before avformat_write_header().
 */
int64_t max_interleave_delta;

/**
 * Allow non-standard and experimental extension
 * @see AVCodecContext.strict_std_compliance
 */
int strict_std_compliance;

/**
 * Flags indicating events happening on the file, a combination of
 * AVFMT_EVENT_FLAG_*.
 *
 * - demuxing: may be set by the demuxer in avformat_open_input(),
 *   avformat_find_stream_info() and av_read_frame(). Flags must be cleared
 *   by the user once the event has been handled.
 * - muxing: may be set by the user after avformat_write_header() to
 *   indicate a user-triggered event.  The muxer will clear the flags for
 *   events it has handled in av_[interleaved]_write_frame().
 */
int event_flags;
/**
 * - demuxing: the demuxer read new metadata from the file and updated
 *   AVFormatContext.metadata accordingly
 * - muxing: the user updated AVFormatContext.metadata and wishes the muxer to
 *   write it into the file
 */
#define AVFMT_EVENT_FLAG_METADATA_UPDATED 0x0001

/**
 * Maximum number of packets to read while waiting for the first timestamp.
 * Decoding only.
 */
int max_ts_probe;

/**
 * Avoid negative timestamps during muxing.
 * Any value of the AVFMT_AVOID_NEG_TS_* constants.
 * Note, this works better when using av_interleaved_write_frame().
 * - muxing: Set by user
 * - demuxing: unused
 */
int avoid_negative_ts;
#define AVFMT_AVOID_NEG_TS_AUTO -1  ///< Enabled when required by target format
#define AVFMT_AVOID_NEG_TS_DISABLED \
  0  ///< Do not shift timestamps even when they are negative.
#define AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE \
  1  ///< Shift timestamps so they are non negative
#define AVFMT_AVOID_NEG_TS_MAKE_ZERO \
  2  ///< Shift timestamps so that they start at 0

/**
 * Transport stream id.
 * This will be moved into demuxer private options. Thus no API/ABI
 * compatibility
 */
int ts_id;

/**
 * Audio preload in microseconds.
 * Note, not all formats support this and unpredictable things may happen if it
 * is used when not supported.
 * - encoding: Set by user
 * - decoding: unused
 */
int audio_preload;

/**
 * Max chunk time in microseconds.
 * Note, not all formats support this and unpredictable things may happen if it
 * is used when not supported.
 * - encoding: Set by user
 * - decoding: unused
 */
int max_chunk_duration;

/**
 * Max chunk size in bytes
 * Note, not all formats support this and unpredictable things may happen if it
 * is used when not supported.
 * - encoding: Set by user
 * - decoding: unused
 */
int max_chunk_size;

/**
 * forces the use of wallclock timestamps as pts/dts of packets
 * This has undefined results in the presence of B frames.
 * - encoding: unused
 * - decoding: Set by user
 */
int use_wallclock_as_timestamps;

/**
 * avio flags, used to force AVIO_FLAG_DIRECT.
 * - encoding: unused
 * - decoding: Set by user
 */
int avio_flags;

/**
 * The duration field can be estimated through various ways, and this field can
 * be used to know how the duration was estimated.
 * - encoding: unused
 * - decoding: Read by user
 */
enum AVDurationEstimationMethod duration_estimation_method;

/**
 * Skip initial bytes when opening stream
 * - encoding: unused
 * - decoding: Set by user
 */
int64_t skip_initial_bytes;

/**
 * Correct single timestamp overflows
 * - encoding: unused
 * - decoding: Set by user
 */
unsigned int correct_ts_overflow;

/**
 * Force seeking to any (also non key) frames.
 * - encoding: unused
 * - decoding: Set by user
 */
int seek2any;

/**
 * Flush the I/O context after each packet.
 * - encoding: Set by user
 * - decoding: unused
 */
int flush_packets;

/**
 * format probing score.
 * The maximal score is AVPROBE_SCORE_MAX, its set when the demuxer probes
 * the format.
 * - encoding: unused
 * - decoding: set by avformat, read by user
 */
int probe_score;

/**
 * Maximum number of bytes read from input in order to identify the
 * \ref AVInputFormat "input format". Only used when the format is not set
 * explicitly by the caller.
 *
 * Demuxing only, set by the caller before avformat_open_input().
 *
 * @sa probesize
 */
int format_probesize;

/**
 * ',' separated list of allowed decoders.
 * If NULL then all are allowed
 * - encoding: unused
 * - decoding: set by user
 */
char *codec_whitelist;

/**
 * ',' separated list of allowed demuxers.
 * If NULL then all are allowed
 * - encoding: unused
 * - decoding: set by user
 */
char *format_whitelist;

/**
 * IO repositioned flag.
 * This is set by avformat when the underlaying IO context read pointer
 * is repositioned, for example when doing byte based seeking.
 * Demuxers can use the flag to detect such changes.
 */
int io_repositioned;

/**
 * Forced video codec.
 * This allows forcing a specific decoder, even when there are multiple with
 * the same codec_id.
 * Demuxing: Set by user
 */
const AVCodec *video_codec;

/**
 * Forced audio codec.
 * This allows forcing a specific decoder, even when there are multiple with
 * the same codec_id.
 * Demuxing: Set by user
 */
const AVCodec *audio_codec;

/**
 * Forced subtitle codec.
 * This allows forcing a specific decoder, even when there are multiple with
 * the same codec_id.
 * Demuxing: Set by user
 */
const AVCodec *subtitle_codec;

/**
 * Forced data codec.
 * This allows forcing a specific decoder, even when there are multiple with
 * the same codec_id.
 * Demuxing: Set by user
 */
const AVCodec *data_codec;

/**
 * Number of bytes to be written as padding in a metadata header.
 * Demuxing: Unused.
 * Muxing: Set by user via av_format_set_metadata_header_padding.
 */
int metadata_header_padding;

/**
 * User data.
 * This is a place for some private data of the user.
 */
void *opaque;

/**
 * Callback used by devices to communicate with application.
 */
av_format_control_message control_message_cb;

/**
 * Output timestamp offset, in microseconds.
 * Muxing: set by user
 */
int64_t output_ts_offset;

/**
 * dump format separator.
 * can be ", " or "\n      " or anything else
 * - muxing: Set by user.
 * - demuxing: Set by user.
 */
uint8_t *dump_separator;

/**
 * Forced Data codec_id.
 * Demuxing: Set by user.
 */
enum AVCodecID data_codec_id;

/**
 * ',' separated list of allowed protocols.
 * - encoding: unused
 * - decoding: set by user
 */
char *protocol_whitelist;

/**
 * A callback for opening new IO streams.
 *
 * Whenever a muxer or a demuxer needs to open an IO stream (typically from
 * avformat_open_input() for demuxers, but for certain formats can happen at
 * other times as well), it will call this callback to obtain an IO context.
 *
 * @param s the format context
 * @param pb on success, the newly opened IO context should be returned here
 * @param url the url to open
 * @param flags a combination of AVIO_FLAG_*
 * @param options a dictionary of additional options, with the same
 *                semantics as in avio_open2()
 * @return 0 on success, a negative AVERROR code on failure
 *
 * @note Certain muxers and demuxers do nesting, i.e. they open one or more
 * additional internal format contexts. Thus the AVFormatContext pointer
 * passed to this callback may be different from the one facing the caller.
 * It will, however, have the same 'opaque' field.
 */
int (*io_open)(struct AVFormatContext *s, AVIOContext **pb, const char *url,
               int flags, AVDictionary **options);

/**
 * A callback for closing the streams opened with AVFormatContext.io_open().
 */
void (*io_close)(struct AVFormatContext *s, AVIOContext *pb);

/**
 * ',' separated list of disallowed protocols.
 * - encoding: unused
 * - decoding: set by user
 */
char *protocol_blacklist;

/**
 * The maximum number of streams.
 * - encoding: unused
 * - decoding: set by user
 */
int max_streams;

/**
 * Skip duration calcuation in estimate_timings_from_pts.
 * - encoding: unused
 * - decoding: set by user
 */
int skip_estimate_duration_from_pts;

/**
 * Maximum number of packets that can be probed
 * - encoding: unused
 * - decoding: set by user
 */
int max_probe_packets;

/**
 * A callback for closing the streams opened with AVFormatContext.io_open().
 *
 * Using this is preferred over io_close, because this can return an error.
 * Therefore this callback is used instead of io_close by the generic
 * libavformat code if io_close is NULL or the default.
 *
 * @param s the format context
 * @param pb IO context to be closed and freed
 * @return 0 on success, a negative AVERROR code on failure
 */
int (*io_close2)(struct AVFormatContext *s, AVIOContext *pb);
}
AVFormatContext;

//AVPacket 
//用于存储已编码的音频和视频数据的结构体。它的主要作用是在解码器、编码器和多媒体文件之间传输压缩数据
typedef struct AVPacket {
    /**
     * A reference to the reference-counted buffer where the packet data is
     * stored.
     * May be NULL, then the packet data is not reference-counted.
     */
    AVBufferRef *buf;
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    //表示此数据包中的音频或视频帧的演示时间戳（Presentation Time Stamp）。它是一个以时间基（time_base）为单位的整数值，表示帧在播放时应显示的时间
    int64_t pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    int64_t dts;
    //此成员是一个指向已编码数据的指针，即存储在此 AVPacket 中的音频或视频帧的编码数据
    uint8_t *data;
    //表示 data 指针中存储的已编码数据的大小（以字节为单位）
    int   size;
    //此成员表示此数据包属于哪个数据流。它是 AVFormatContext 结构体中 streams 数组的索引，用于标识此 AVPacket 中的数据属于音频流、视频流还是其他类型的流。
    int   stream_index;
    /**
     * A combination of AV_PKT_FLAG values
     */
    int   flags;
    /**
     * Additional packet data that can be provided by the container.
     * Packet can contain several types of side information.
     */
    AVPacketSideData *side_data;
    int side_data_elems;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    //表示此数据包中的音频或视频帧的持续时间。它是一个以时间基（time_base）为单位的整数值，表示帧的播放持续时间
    int64_t duration;

    //此成员表示此数据包在输入文件中的字节位置。如果位置未知或不可用，则值为 -1
    int64_t pos;                            ///< byte position in stream, -1 if unknown

    /**
     * for some private data of the user
     */
    void *opaque;

    /**
     * AVBufferRef for free use by the API user. FFmpeg will never check the
     * contents of the buffer ref. FFmpeg calls av_buffer_unref() on it when
     * the packet is unreferenced. av_packet_copy_props() calls create a new
     * reference with av_buffer_ref() for the target packet's opaque_ref field.
     *
     * This is unrelated to the opaque field, although it serves a similar
     * purpose.
     */
    AVBufferRef *opaque_ref;

    /**
     * Time base of the packet's timestamps.
     * In the future, this field may be set on packets output by encoders or
     * demuxers, but its value will be by default ignored on input to decoders
     * or muxers.
     */
    AVRational time_base;
} AVPacket;


//AVInputFormat
//描述输入媒体文件格式的信息
typedef struct AVInputFormat {
    /**
     * A comma separated list of short names for the format. New names
     * may be appended with a minor bump.
     */
    //输入格式的名称，例如 "mp4"、"avi" 等
    const char *name;

    /**
     * Descriptive name for the format, meant to be more human-readable
     * than name. You should use the NULL_IF_CONFIG_SMALL() macro
     * to define it.
     */
    //输入格式的长名称，通常是更详细的描述
    const char *long_name;

    /**
     * Can use flags: AVFMT_NOFILE, AVFMT_NEEDNUMBER, AVFMT_SHOW_IDS,
     * AVFMT_NOTIMESTAMPS, AVFMT_GENERIC_INDEX, AVFMT_TS_DISCONT, AVFMT_NOBINSEARCH,
     * AVFMT_NOGENSEARCH, AVFMT_NO_BYTE_SEEK, AVFMT_SEEK_TO_PTS.
     */
    // 用于指示输入格式的特性。例如，AVFMT_NOFILE 表示该格式不需要文件 I/O（输入/输出）支持
    int flags;

    /**
     * If extensions are defined, then no probe is done. You should
     * usually not use extension format guessing because it is not
     * reliable enough
     */
    //这是一个字符串，包含了此格式的所有可能文件扩展名，由逗号分隔
    const char *extensions;

    //一个指向编解码器标签数组的指针，用于映射本地编解码器 ID 到输入格式
    //的编解码器 ID
    const struct AVCodecTag * const *codec_tag;

    //指向私有类的指针，用于存储输入格式特定的选项
    const AVClass *priv_class; ///< AVClass for the private context

    /**
     * Comma-separated list of mime types.
     * It is used check for matching mime types while probing.
     * @see av_probe_input_format2
     */
    const char *mime_type;

    /*****************************************************************
     * No fields below this line are part of the public API. They
     * may not be used outside of libavformat and can be changed and
     * removed at will.
     * New public fields should be added right above.
     *****************************************************************
     */
    /**
     * Raw demuxers store their codec ID here.
     */
    //用于原始数据的编解码器 ID
    int raw_codec_id;

    /**
     * Size of private data so that it can be allocated in the wrapper.
     */
    //输入格式的私有数据大小，以字节为单位
    int priv_data_size;

    /**
     * Internal flags. See FF_FMT_FLAG_* in internal.h.
     */
    int flags_internal;

    /**
     * Tell if a given file has a chance of being parsed as this format.
     * The buffer provided is guaranteed to be AVPROBE_PADDING_SIZE bytes
     * big so you do not have to check for that unless you need more.
     */
    int (*read_probe)(const AVProbeData *);

    /**
     * Read the format header and initialize the AVFormatContext
     * structure. Return 0 if OK. 'avformat_new_stream' should be
     * called to create new streams.
     */
    int (*read_header)(struct AVFormatContext *);

    /**
     * Read one packet and put it in 'pkt'. pts and flags are also
     * set. 'avformat_new_stream' can be called only if the flag
     * AVFMTCTX_NOHEADER is used and only in the calling thread (not in a
     * background thread).
     * @return 0 on success, < 0 on error.
     *         Upon returning an error, pkt must be unreferenced by the caller.
     */
    int (*read_packet)(struct AVFormatContext *, AVPacket *pkt);

    /**
     * Close the stream. The AVFormatContext and AVStreams are not
     * freed by this function
     */
    int (*read_close)(struct AVFormatContext *);

    /**
     * Seek to a given timestamp relative to the frames in
     * stream component stream_index.
     * @param stream_index Must not be -1.
     * @param flags Selects which direction should be preferred if no exact
     *              match is available.
     * @return >= 0 on success (but not necessarily the new offset)
     */
    int (*read_seek)(struct AVFormatContext *,
                     int stream_index, int64_t timestamp, int flags);

    /**
     * Get the next timestamp in stream[stream_index].time_base units.
     * @return the timestamp or AV_NOPTS_VALUE if an error occurred
     */
    int64_t (*read_timestamp)(struct AVFormatContext *s, int stream_index,
                              int64_t *pos, int64_t pos_limit);

    /**
     * Start/resume playing - only meaningful if using a network-based format
     * (RTSP).
     */
    int (*read_play)(struct AVFormatContext *);

    /**
     * Pause playing - only meaningful if using a network-based format
     * (RTSP).
     */
    int (*read_pause)(struct AVFormatContext *);

    /**
     * Seek to timestamp ts.
     * Seeking will be done so that the point from which all active streams
     * can be presented successfully will be closest to ts and within min/max_ts.
     * Active streams are all streams that have AVStream.discard < AVDISCARD_ALL.
     */
    int (*read_seek2)(struct AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);

    /**
     * Returns device list with it properties.
     * @see avdevice_list_devices() for more details.
     */
    int (*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);

} AVInputFormat;


//libavutil/log.h
// AVClass 
//提供类的元数据和日志功能。它包含了一些成员变量
/**
 * Describe the class of an AVClass context structure. That is an
 * arbitrary struct of which the first field is a pointer to an
 * AVClass struct (e.g. AVCodecContext, AVFormatContext etc.).
 */
typedef struct AVClass {
    /**
     * The name of the class; usually it is the same name as the
     * context structure type to which the AVClass is associated.
     */
    //类的名称，用于标识和描述类
    const char* class_name;

    /**
     * A pointer to a function which returns the name of a context
     * instance ctx associated with the class.
     */
    const char* (*item_name)(void* ctx);

    /**
     * a pointer to the first option specified in the class if any or NULL
     *
     * @see av_set_default_options()
     */
    //指向一个AVOption 结构体数组的指针，用于存储类的选项。这些选项用于配置类的实例
    const struct AVOption *option;

    /**
     * LIBAVUTIL_VERSION with which this structure was created.
     * This is used to allow fields to be added without requiring major
     * version bumps everywhere.
     */

    int version;

    /**
     * Offset in the structure where log_level_offset is stored.
     * 0 means there is no such variable
     */
    //类实例中日志级别的偏移量，以字节为单位。这个偏移量用于在类实例中找到日志级别的整数值
    int log_level_offset_offset;

    /**
     * Offset in the structure where a pointer to the parent context for
     * logging is stored. For example a decoder could pass its AVCodecContext
     * to eval as such a parent context, which an av_log() implementation
     * could then leverage to display the parent context.
     * The offset can be NULL.
     */
    //类实例中父日志上下文的偏移量，以字节为单位。这个偏移量用于在类实例中找到父日志上下文的指针
    int parent_log_context_offset;

    /**
     * Category used for visualization (like color)
     * This is only set if the category is equal for all objects using this class.
     * available since version (51 << 16 | 56 << 8 | 100)
     */
    AVClassCategory category;

    /**
     * Callback to return the category.
     * available since version (51 << 16 | 59 << 8 | 100)
     */
    AVClassCategory (*get_category)(void* ctx);

    /**
     * Callback to return the supported/allowed ranges.
     * available since version (52.12)
     */
    int (*query_ranges)(struct AVOptionRanges **, void *obj, const char *key, int flags);

    /**
     * Return next AVOptions-enabled child or NULL
     */
    void* (*child_next)(void *obj, void *prev);

    /**
     * Iterate over the AVClasses corresponding to potential AVOptions-enabled
     * children.
     *
     * @param iter pointer to opaque iteration state. The caller must initialize
     *             *iter to NULL before the first call.
     * @return AVClass for the next AVOptions-enabled child or NULL if there are
     *         no more such children.
     *
     * @note The difference between child_next and this is that child_next
     *       iterates over _already existing_ objects, while child_class_iterate
     *       iterates over _all possible_ children.
     */
    const struct AVClass* (*child_class_iterate)(void **iter);
} AVClass;

//AVStream 
//描述一个媒体文件中的音频、视频或字幕流
/**
 * Stream structure.
 * New fields can be added to the end with minor version bumps.
 * Removal, reordering and changes to existing fields require a major
 * version bump.
 * sizeof(AVStream) must not be used outside libav*.
 */
typedef struct AVStream {
#if FF_API_AVSTREAM_CLASS
    /**
     * A class for @ref avoptions. Set on stream creation.
     */
    const AVClass *av_class;
#endif

    //流的索引，用于标识流在媒体文件中的位置
    int index;    /**< stream index in AVFormatContext */
    /**
     * Format-specific stream ID.
     * decoding: set by libavformat
     * encoding: set by the user, replaced by libavformat if left unset
     */
    int id;

    void *priv_data;

    /**
     * This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented.
     *
     * decoding: set by libavformat
     * encoding: May be set by the caller before avformat_write_header() to
     *           provide a hint to the muxer about the desired timebase. In
     *           avformat_write_header(), the muxer will overwrite this field
     *           with the timebase that will actually be used for the timestamps
     *           written into the file (which may or may not be related to the
     *           user-provided one, depending on the format).
     */
    AVRational time_base;

    /**
     * Decoding: pts of the first frame of the stream in presentation order, in stream time base.
     * Only set this if you are absolutely 100% sure that the value you set
     * it to really is the pts of the first frame.
     * This may be undefined (AV_NOPTS_VALUE).
     * @note The ASF header does NOT contain a correct start_time the ASF
     * demuxer must NOT set this.
     */
    int64_t start_time;

    /**
     * Decoding: duration of the stream, in stream time base.
     * If a source file does not specify a duration, but does specify
     * a bitrate, this value will be estimated from bitrate and file size.
     *
     * Encoding: May be set by the caller before avformat_write_header() to
     * provide a hint to the muxer about the estimated duration.
     */
    int64_t duration;

    //流中的总帧数
    int64_t nb_frames;                 ///< number of frames in this stream if known or 0

    /**
     * Stream disposition - a combination of AV_DISPOSITION_* flags.
     * - demuxing: set by libavformat when creating the stream or in
     *             avformat_find_stream_info().
     * - muxing: may be set by the caller before avformat_write_header().
     */
    //流的属性，例如是否为默认流、是否为附加流等
    int disposition;

    enum AVDiscard discard; ///< Selects which packets can be discarded at will and do not need to be demuxed.

    /**
     * sample aspect ratio (0 if unknown)
     * - encoding: Set by user.
     * - decoding: Set by libavformat.
     */
    AVRational sample_aspect_ratio;

    AVDictionary *metadata;

    /**
     * Average framerate
     *
     * - demuxing: May be set by libavformat when creating the stream or in
     *             avformat_find_stream_info().
     * - muxing: May be set by the caller before avformat_write_header().
     */
    //流的平均帧率。它是一个有理数，表示每秒钟的帧数
    AVRational avg_frame_rate;

    /**
     * For streams with AV_DISPOSITION_ATTACHED_PIC disposition, this packet
     * will contain the attached picture.
     *
     * decoding: set by libavformat, must not be modified by the caller.
     * encoding: unused
     */
    AVPacket attached_pic;

    /**
     * An array of side data that applies to the whole stream (i.e. the
     * container does not allow it to change between packets).
     *
     * There may be no overlap between the side data in this array and side data
     * in the packets. I.e. a given side data is either exported by the muxer
     * (demuxing) / set by the caller (muxing) in this array, then it never
     * appears in the packets, or the side data is exported / sent through
     * the packets (always in the first packet where the value becomes known or
     * changes), then it does not appear in this array.
     *
     * - demuxing: Set by libavformat when the stream is created.
     * - muxing: May be set by the caller before avformat_write_header().
     *
     * Freed by libavformat in avformat_free_context().
     *
     * @see av_format_inject_global_side_data()
     */
    AVPacketSideData *side_data;
    /**
     * The number of elements in the AVStream.side_data array.
     */
    int            nb_side_data;

    /**
     * Flags indicating events happening on the stream, a combination of
     * AVSTREAM_EVENT_FLAG_*.
     *
     * - demuxing: may be set by the demuxer in avformat_open_input(),
     *   avformat_find_stream_info() and av_read_frame(). Flags must be cleared
     *   by the user once the event has been handled.
     * - muxing: may be set by the user after avformat_write_header(). to
     *   indicate a user-triggered event.  The muxer will clear the flags for
     *   events it has handled in av_[interleaved]_write_frame().
     */
    int event_flags;
/**
 * - demuxing: the demuxer read new metadata from the file and updated
 *     AVStream.metadata accordingly
 * - muxing: the user updated AVStream.metadata and wishes the muxer to write
 *     it into the file
 */
#define AVSTREAM_EVENT_FLAG_METADATA_UPDATED 0x0001
/**
 * - demuxing: new packets for this stream were read from the file. This
 *   event is informational only and does not guarantee that new packets
 *   for this stream will necessarily be returned from av_read_frame().
 */
#define AVSTREAM_EVENT_FLAG_NEW_PACKETS (1 << 1)

    /**
     * Real base framerate of the stream.
     * This is the lowest framerate with which all timestamps can be
     * represented accurately (it is the least common multiple of all
     * framerates in the stream). Note, this value is just a guess!
     * For example, if the time base is 1/90000 and all frames have either
     * approximately 3600 or 1800 timer ticks, then r_frame_rate will be 50/1.
     */
    AVRational r_frame_rate;

    /**
     * Codec parameters associated with this stream. Allocated and freed by
     * libavformat in avformat_new_stream() and avformat_free_context()
     * respectively.
     *
     * - demuxing: filled by libavformat on stream creation or in
     *             avformat_find_stream_info()
     * - muxing: filled by the caller before avformat_write_header()
     */
    AVCodecParameters *codecpar;

    /**
     * Number of bits in timestamps. Used for wrapping control.
     *
     * - demuxing: set by libavformat
     * - muxing: set by libavformat
     *
     */
    int pts_wrap_bits;
} AVStream;


// AVCodecContext 
//描述一个音频或视频流的编解码器上下文
/**
 * main external API structure.
 * New fields can be added to the end with minor version bumps.
 * Removal, reordering and changes to existing fields require a major
 * version bump.
 * You can use AVOptions (av_opt* / av_set/get*()) to access these fields from user
 * applications.
 * The name string for AVOptions options matches the associated command line
 * parameter name and can be found in libavcodec/options_table.h
 * The AVOption/command line parameter names differ in some cases from the C
 * structure field names for historic reasons or brevity.
 * sizeof(AVCodecContext) must not be used outside libav*.
 */
typedef struct AVCodecContext {
    /**
     * information on struct for av_log
     * - set by avcodec_alloc_context3
     */
    // 指向一个 AVClass 结构体的指针，用于存储编解码器上下文的类信息
    const AVClass *av_class;
    int log_level_offset;

    //流的媒体类型，例如音频、视频或字幕
    enum AVMediaType codec_type; /* see AVMEDIA_TYPE_xxx */
    // 指向一个 AVCodec 结构体的指针，用于存储编解码器的信息
    const struct AVCodec  *codec;

    //编解码器的 ID，用于标识特定的编解码器
    enum AVCodecID     codec_id; /* see AV_CODEC_ID_xxx */

    /**
     * fourcc (LSB first, so "ABCD" -> ('D'<<24) + ('C'<<16) + ('B'<<8) + 'A').
     * This is used to work around some encoder bugs.
     * A demuxer should set this to what is stored in the field used to identify the codec.
     * If there are multiple such fields in a container then the demuxer should choose the one
     * which maximizes the information about the used codec.
     * If the codec tag field in a container is larger than 32 bits then the demuxer should
     * remap the longer ID to 32 bits with a table or other structure. Alternatively a new
     * extra_codec_tag + size could be added but for this a clear advantage must be demonstrated
     * first.
     * - encoding: Set by user, if not then the default based on codec_id will be used.
     * - decoding: Set by user, will be converted to uppercase by libavcodec during init.
     */
    unsigned int codec_tag;

    void *priv_data;

    /**
     * Private context used for internal data.
     *
     * Unlike priv_data, this is not codec-specific. It is used in general
     * libavcodec functions.
     */
    struct AVCodecInternal *internal;

    /**
     * Private data of the user, can be used to carry app specific stuff.
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    void *opaque;

    /**
     * the average bitrate
     * - encoding: Set by user; unused for constant quantizer encoding.
     * - decoding: Set by user, may be overwritten by libavcodec
     *             if this info is available in the stream
     */
    int64_t bit_rate;

    /**
     * number of bits the bitstream is allowed to diverge from the reference.
     *           the reference can be CBR (for CBR pass1) or VBR (for pass2)
     * - encoding: Set by user; unused for constant quantizer encoding.
     * - decoding: unused
     */
    int bit_rate_tolerance;

    /**
     * Global quality for codecs which cannot change it per frame.
     * This should be proportional to MPEG-1/2/4 qscale.
     * - encoding: Set by user.
     * - decoding: unused
     */
    int global_quality;

    /**
     * - encoding: Set by user.
     * - decoding: unused
     */
    int compression_level;
#define FF_COMPRESSION_DEFAULT -1

    /**
     * AV_CODEC_FLAG_*.
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int flags;

    /**
     * AV_CODEC_FLAG2_*
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int flags2;

    /**
     * some codecs need / can use extradata like Huffman tables.
     * MJPEG: Huffman tables
     * rv10: additional flags
     * MPEG-4: global headers (they can be in the bitstream or here)
     * The allocated memory should be AV_INPUT_BUFFER_PADDING_SIZE bytes larger
     * than extradata_size to avoid problems if it is read with the bitstream reader.
     * The bytewise contents of extradata must not depend on the architecture or CPU endianness.
     * Must be allocated with the av_malloc() family of functions.
     * - encoding: Set/allocated/freed by libavcodec.
     * - decoding: Set/allocated/freed by user.
     */
    uint8_t *extradata;
    int extradata_size;

    /**
     * This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identically 1.
     * This often, but not always is the inverse of the frame rate or field rate
     * for video. 1/time_base is not the average frame rate if the frame rate is not
     * constant.
     *
     * Like containers, elementary streams also can store timestamps, 1/time_base
     * is the unit in which these timestamps are specified.
     * As example of such codec time base see ISO/IEC 14496-2:2001(E)
     * vop_time_increment_resolution and fixed_vop_rate
     * (fixed_vop_rate == 0 implies that it is different from the framerate)
     *
     * - encoding: MUST be set by user.
     * - decoding: the use of this field for decoding is deprecated.
     *             Use framerate instead.
     */
    AVRational time_base;

    /**
     * For some codecs, the time base is closer to the field rate than the frame rate.
     * Most notably, H.264 and MPEG-2 specify time_base as half of frame duration
     * if no telecine is used ...
     *
     * Set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2.
     */
    int ticks_per_frame;

    /**
     * Codec delay.
     *
     * Encoding: Number of frames delay there will be from the encoder input to
     *           the decoder output. (we assume the decoder matches the spec)
     * Decoding: Number of frames delay in addition to what a standard decoder
     *           as specified in the spec would produce.
     *
     * Video:
     *   Number of frames the decoded output will be delayed relative to the
     *   encoded input.
     *
     * Audio:
     *   For encoding, this field is unused (see initial_padding).
     *
     *   For decoding, this is the number of samples the decoder needs to
     *   output before the decoder's output is valid. When seeking, you should
     *   start decoding this many samples prior to your desired seek point.
     *
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    int delay;


    /* video only */
    /**
     * picture width / height.
     *
     * @note Those fields may not match the values of the last
     * AVFrame output by avcodec_receive_frame() due frame
     * reordering.
     *
     * - encoding: MUST be set by user.
     * - decoding: May be set by the user before opening the decoder if known e.g.
     *             from the container. Some decoders will require the dimensions
     *             to be set by the caller. During decoding, the decoder may
     *             overwrite those values as required while parsing the data.
     */
    int width, height;

    /**
     * Bitstream width / height, may be different from width/height e.g. when
     * the decoded frame is cropped before being output or lowres is enabled.
     *
     * @note Those field may not match the value of the last
     * AVFrame output by avcodec_receive_frame() due frame
     * reordering.
     *
     * - encoding: unused
     * - decoding: May be set by the user before opening the decoder if known
     *             e.g. from the container. During decoding, the decoder may
     *             overwrite those values as required while parsing the data.
     */
    int coded_width, coded_height;

    /**
     * the number of pictures in a group of pictures, or 0 for intra_only
     * - encoding: Set by user.
     * - decoding: unused
     */
    int gop_size;

    /**
     * Pixel format, see AV_PIX_FMT_xxx.
     * May be set by the demuxer if known from headers.
     * May be overridden by the decoder if it knows better.
     *
     * @note This field may not match the value of the last
     * AVFrame output by avcodec_receive_frame() due frame
     * reordering.
     *
     * - encoding: Set by user.
     * - decoding: Set by user if known, overridden by libavcodec while
     *             parsing the data.
     */
    enum AVPixelFormat pix_fmt;

    /**
     * If non NULL, 'draw_horiz_band' is called by the libavcodec
     * decoder to draw a horizontal band. It improves cache usage. Not
     * all codecs can do that. You must check the codec capabilities
     * beforehand.
     * When multithreading is used, it may be called from multiple threads
     * at the same time; threads might draw different parts of the same AVFrame,
     * or multiple AVFrames, and there is no guarantee that slices will be drawn
     * in order.
     * The function is also used by hardware acceleration APIs.
     * It is called at least once during frame decoding to pass
     * the data needed for hardware render.
     * In that mode instead of pixel data, AVFrame points to
     * a structure specific to the acceleration API. The application
     * reads the structure and can change some fields to indicate progress
     * or mark state.
     * - encoding: unused
     * - decoding: Set by user.
     * @param height the height of the slice
     * @param y the y position of the slice
     * @param type 1->top field, 2->bottom field, 3->frame
     * @param offset offset into the AVFrame.data from which the slice should be read
     */
    void (*draw_horiz_band)(struct AVCodecContext *s,
                            const AVFrame *src, int offset[AV_NUM_DATA_POINTERS],
                            int y, int type, int height);

    /**
     * Callback to negotiate the pixel format. Decoding only, may be set by the
     * caller before avcodec_open2().
     *
     * Called by some decoders to select the pixel format that will be used for
     * the output frames. This is mainly used to set up hardware acceleration,
     * then the provided format list contains the corresponding hwaccel pixel
     * formats alongside the "software" one. The software pixel format may also
     * be retrieved from \ref sw_pix_fmt.
     *
     * This callback will be called when the coded frame properties (such as
     * resolution, pixel format, etc.) change and more than one output format is
     * supported for those new properties. If a hardware pixel format is chosen
     * and initialization for it fails, the callback may be called again
     * immediately.
     *
     * This callback may be called from different threads if the decoder is
     * multi-threaded, but not from more than one thread simultaneously.
     *
     * @param fmt list of formats which may be used in the current
     *            configuration, terminated by AV_PIX_FMT_NONE.
     * @warning Behavior is undefined if the callback returns a value other
     *          than one of the formats in fmt or AV_PIX_FMT_NONE.
     * @return the chosen format or AV_PIX_FMT_NONE
     */
    enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat * fmt);

    /**
     * maximum number of B-frames between non-B-frames
     * Note: The output will be delayed by max_b_frames+1 relative to the input.
     * - encoding: Set by user.
     * - decoding: unused
     */
    int max_b_frames;

    /**
     * qscale factor between IP and B-frames
     * If > 0 then the last P-frame quantizer will be used (q= lastp_q*factor+offset).
     * If < 0 then normal ratecontrol will be done (q= -normal_q*factor+offset).
     * - encoding: Set by user.
     * - decoding: unused
     */
    float b_quant_factor;

    /**
     * qscale offset between IP and B-frames
     * - encoding: Set by user.
     * - decoding: unused
     */
    float b_quant_offset;

    /**
     * Size of the frame reordering buffer in the decoder.
     * For MPEG-2 it is 1 IPB or 0 low delay IP.
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    int has_b_frames;

    /**
     * qscale factor between P- and I-frames
     * If > 0 then the last P-frame quantizer will be used (q = lastp_q * factor + offset).
     * If < 0 then normal ratecontrol will be done (q= -normal_q*factor+offset).
     * - encoding: Set by user.
     * - decoding: unused
     */
    float i_quant_factor;

    /**
     * qscale offset between P and I-frames
     * - encoding: Set by user.
     * - decoding: unused
     */
    float i_quant_offset;

    /**
     * luminance masking (0-> disabled)
     * - encoding: Set by user.
     * - decoding: unused
     */
    float lumi_masking;

    /**
     * temporary complexity masking (0-> disabled)
     * - encoding: Set by user.
     * - decoding: unused
     */
    float temporal_cplx_masking;

    /**
     * spatial complexity masking (0-> disabled)
     * - encoding: Set by user.
     * - decoding: unused
     */
    float spatial_cplx_masking;

    /**
     * p block masking (0-> disabled)
     * - encoding: Set by user.
     * - decoding: unused
     */
    float p_masking;

    /**
     * darkness masking (0-> disabled)
     * - encoding: Set by user.
     * - decoding: unused
     */
    float dark_masking;

    /**
     * slice count
     * - encoding: Set by libavcodec.
     * - decoding: Set by user (or 0).
     */
    int slice_count;

    /**
     * slice offsets in the frame in bytes
     * - encoding: Set/allocated by libavcodec.
     * - decoding: Set/allocated by user (or NULL).
     */
    int *slice_offset;

    /**
     * sample aspect ratio (0 if unknown)
     * That is the width of a pixel divided by the height of the pixel.
     * Numerator and denominator must be relatively prime and smaller than 256 for some video standards.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    AVRational sample_aspect_ratio;

    /**
     * motion estimation comparison function
     * - encoding: Set by user.
     * - decoding: unused
     */
    int me_cmp;
    /**
     * subpixel motion estimation comparison function
     * - encoding: Set by user.
     * - decoding: unused
     */
    int me_sub_cmp;
    /**
     * macroblock comparison function (not supported yet)
     * - encoding: Set by user.
     * - decoding: unused
     */
    int mb_cmp;
    /**
     * interlaced DCT comparison function
     * - encoding: Set by user.
     * - decoding: unused
     */
    int ildct_cmp;
#define FF_CMP_SAD          0
#define FF_CMP_SSE          1
#define FF_CMP_SATD         2
#define FF_CMP_DCT          3
#define FF_CMP_PSNR         4
#define FF_CMP_BIT          5
#define FF_CMP_RD           6
#define FF_CMP_ZERO         7
#define FF_CMP_VSAD         8
#define FF_CMP_VSSE         9
#define FF_CMP_NSSE         10
#define FF_CMP_W53          11
#define FF_CMP_W97          12
#define FF_CMP_DCTMAX       13
#define FF_CMP_DCT264       14
#define FF_CMP_MEDIAN_SAD   15
#define FF_CMP_CHROMA       256

    /**
     * ME diamond size & shape
     * - encoding: Set by user.
     * - decoding: unused
     */
    int dia_size;

    /**
     * amount of previous MV predictors (2a+1 x 2a+1 square)
     * - encoding: Set by user.
     * - decoding: unused
     */
    int last_predictor_count;

    /**
     * motion estimation prepass comparison function
     * - encoding: Set by user.
     * - decoding: unused
     */
    int me_pre_cmp;

    /**
     * ME prepass diamond size & shape
     * - encoding: Set by user.
     * - decoding: unused
     */
    int pre_dia_size;

    /**
     * subpel ME quality
     * - encoding: Set by user.
     * - decoding: unused
     */
    int me_subpel_quality;

    /**
     * maximum motion estimation search range in subpel units
     * If 0 then no limit.
     *
     * - encoding: Set by user.
     * - decoding: unused
     */
    int me_range;

    /**
     * slice flags
     * - encoding: unused
     * - decoding: Set by user.
     */
    int slice_flags;
#define SLICE_FLAG_CODED_ORDER    0x0001 ///< draw_horiz_band() is called in coded order instead of display
#define SLICE_FLAG_ALLOW_FIELD    0x0002 ///< allow draw_horiz_band() with field slices (MPEG-2 field pics)
#define SLICE_FLAG_ALLOW_PLANE    0x0004 ///< allow draw_horiz_band() with 1 component at a time (SVQ1)

    /**
     * macroblock decision mode
     * - encoding: Set by user.
     * - decoding: unused
     */
    int mb_decision;
#define FF_MB_DECISION_SIMPLE 0        ///< uses mb_cmp
#define FF_MB_DECISION_BITS   1        ///< chooses the one which needs the fewest bits
#define FF_MB_DECISION_RD     2        ///< rate distortion

    /**
     * custom intra quantization matrix
     * Must be allocated with the av_malloc() family of functions, and will be freed in
     * avcodec_free_context().
     * - encoding: Set/allocated by user, freed by libavcodec. Can be NULL.
     * - decoding: Set/allocated/freed by libavcodec.
     */
    uint16_t *intra_matrix;

    /**
     * custom inter quantization matrix
     * Must be allocated with the av_malloc() family of functions, and will be freed in
     * avcodec_free_context().
     * - encoding: Set/allocated by user, freed by libavcodec. Can be NULL.
     * - decoding: Set/allocated/freed by libavcodec.
     */
    uint16_t *inter_matrix;

    /**
     * precision of the intra DC coefficient - 8
     * - encoding: Set by user.
     * - decoding: Set by libavcodec
     */
    int intra_dc_precision;

    /**
     * Number of macroblock rows at the top which are skipped.
     * - encoding: unused
     * - decoding: Set by user.
     */
    int skip_top;

    /**
     * Number of macroblock rows at the bottom which are skipped.
     * - encoding: unused
     * - decoding: Set by user.
     */
    int skip_bottom;

    /**
     * minimum MB Lagrange multiplier
     * - encoding: Set by user.
     * - decoding: unused
     */
    int mb_lmin;

    /**
     * maximum MB Lagrange multiplier
     * - encoding: Set by user.
     * - decoding: unused
     */
    int mb_lmax;

    /**
     * - encoding: Set by user.
     * - decoding: unused
     */
    int bidir_refine;

    /**
     * minimum GOP size
     * - encoding: Set by user.
     * - decoding: unused
     */
    int keyint_min;

    /**
     * number of reference frames
     * - encoding: Set by user.
     * - decoding: Set by lavc.
     */
    int refs;

    /**
     * Note: Value depends upon the compare function used for fullpel ME.
     * - encoding: Set by user.
     * - decoding: unused
     */
    int mv0_threshold;

    /**
     * Chromaticity coordinates of the source primaries.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorPrimaries color_primaries;

    /**
     * Color Transfer Characteristic.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorTransferCharacteristic color_trc;

    /**
     * YUV colorspace type.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorSpace colorspace;

    /**
     * MPEG vs JPEG YUV range.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorRange color_range;

    /**
     * This defines the location of chroma samples.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVChromaLocation chroma_sample_location;

    /**
     * Number of slices.
     * Indicates number of picture subdivisions. Used for parallelized
     * decoding.
     * - encoding: Set by user
     * - decoding: unused
     */
    int slices;

    /** Field order
     * - encoding: set by libavcodec
     * - decoding: Set by user.
     */
    enum AVFieldOrder field_order;

    /* audio only */
    int sample_rate; ///< samples per second

#if FF_API_OLD_CHANNEL_LAYOUT
    /**
     * number of audio channels
     * @deprecated use ch_layout.nb_channels
     */
    attribute_deprecated
    int channels;
#endif

    /**
     * audio sample format
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    enum AVSampleFormat sample_fmt;  ///< sample format

    /* The following data should not be initialized. */
    /**
     * Number of samples per channel in an audio frame.
     *
     * - encoding: set by libavcodec in avcodec_open2(). Each submitted frame
     *   except the last must contain exactly frame_size samples per channel.
     *   May be 0 when the codec has AV_CODEC_CAP_VARIABLE_FRAME_SIZE set, then the
     *   frame size is not restricted.
     * - decoding: may be set by some decoders to indicate constant frame size
     */
    int frame_size;

    /**
     * Frame counter, set by libavcodec.
     *
     * - decoding: total number of frames returned from the decoder so far.
     * - encoding: total number of frames passed to the encoder so far.
     *
     *   @note the counter is not incremented if encoding/decoding resulted in
     *   an error.
     */
    int frame_number;

    /**
     * number of bytes per packet if constant and known or 0
     * Used by some WAV based audio codecs.
     */
    int block_align;

    /**
     * Audio cutoff bandwidth (0 means "automatic")
     * - encoding: Set by user.
     * - decoding: unused
     */
    int cutoff;

#if FF_API_OLD_CHANNEL_LAYOUT
    /**
     * Audio channel layout.
     * - encoding: set by user.
     * - decoding: set by user, may be overwritten by libavcodec.
     * @deprecated use ch_layout
     */
    attribute_deprecated
    uint64_t channel_layout;

    /**
     * Request decoder to use this channel layout if it can (0 for default)
     * - encoding: unused
     * - decoding: Set by user.
     * @deprecated use "downmix" codec private option
     */
    attribute_deprecated
    uint64_t request_channel_layout;
#endif

    /**
     * Type of service that the audio stream conveys.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    enum AVAudioServiceType audio_service_type;

    /**
     * desired sample format
     * - encoding: Not used.
     * - decoding: Set by user.
     * Decoder will decode to this format if it can.
     */
    enum AVSampleFormat request_sample_fmt;

    /**
     * This callback is called at the beginning of each frame to get data
     * buffer(s) for it. There may be one contiguous buffer for all the data or
     * there may be a buffer per each data plane or anything in between. What
     * this means is, you may set however many entries in buf[] you feel necessary.
     * Each buffer must be reference-counted using the AVBuffer API (see description
     * of buf[] below).
     *
     * The following fields will be set in the frame before this callback is
     * called:
     * - format
     * - width, height (video only)
     * - sample_rate, channel_layout, nb_samples (audio only)
     * Their values may differ from the corresponding values in
     * AVCodecContext. This callback must use the frame values, not the codec
     * context values, to calculate the required buffer size.
     *
     * This callback must fill the following fields in the frame:
     * - data[]
     * - linesize[]
     * - extended_data:
     *   * if the data is planar audio with more than 8 channels, then this
     *     callback must allocate and fill extended_data to contain all pointers
     *     to all data planes. data[] must hold as many pointers as it can.
     *     extended_data must be allocated with av_malloc() and will be freed in
     *     av_frame_unref().
     *   * otherwise extended_data must point to data
     * - buf[] must contain one or more pointers to AVBufferRef structures. Each of
     *   the frame's data and extended_data pointers must be contained in these. That
     *   is, one AVBufferRef for each allocated chunk of memory, not necessarily one
     *   AVBufferRef per data[] entry. See: av_buffer_create(), av_buffer_alloc(),
     *   and av_buffer_ref().
     * - extended_buf and nb_extended_buf must be allocated with av_malloc() by
     *   this callback and filled with the extra buffers if there are more
     *   buffers than buf[] can hold. extended_buf will be freed in
     *   av_frame_unref().
     *
     * If AV_CODEC_CAP_DR1 is not set then get_buffer2() must call
     * avcodec_default_get_buffer2() instead of providing buffers allocated by
     * some other means.
     *
     * Each data plane must be aligned to the maximum required by the target
     * CPU.
     *
     * @see avcodec_default_get_buffer2()
     *
     * Video:
     *
     * If AV_GET_BUFFER_FLAG_REF is set in flags then the frame may be reused
     * (read and/or written to if it is writable) later by libavcodec.
     *
     * avcodec_align_dimensions2() should be used to find the required width and
     * height, as they normally need to be rounded up to the next multiple of 16.
     *
     * Some decoders do not support linesizes changing between frames.
     *
     * If frame multithreading is used, this callback may be called from a
     * different thread, but not from more than one at once. Does not need to be
     * reentrant.
     *
     * @see avcodec_align_dimensions2()
     *
     * Audio:
     *
     * Decoders request a buffer of a particular size by setting
     * AVFrame.nb_samples prior to calling get_buffer2(). The decoder may,
     * however, utilize only part of the buffer by setting AVFrame.nb_samples
     * to a smaller value in the output frame.
     *
     * As a convenience, av_samples_get_buffer_size() and
     * av_samples_fill_arrays() in libavutil may be used by custom get_buffer2()
     * functions to find the required data size and to fill data pointers and
     * linesize. In AVFrame.linesize, only linesize[0] may be set for audio
     * since all planes must be the same size.
     *
     * @see av_samples_get_buffer_size(), av_samples_fill_arrays()
     *
     * - encoding: unused
     * - decoding: Set by libavcodec, user can override.
     */
    int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

    /* - encoding parameters */
    float qcompress;  ///< amount of qscale change between easy & hard scenes (0.0-1.0)
    float qblur;      ///< amount of qscale smoothing over time (0.0-1.0)

    /**
     * minimum quantizer
     * - encoding: Set by user.
     * - decoding: unused
     */
    int qmin;

    /**
     * maximum quantizer
     * - encoding: Set by user.
     * - decoding: unused
     */
    int qmax;

    /**
     * maximum quantizer difference between frames
     * - encoding: Set by user.
     * - decoding: unused
     */
    int max_qdiff;

    /**
     * decoder bitstream buffer size
     * - encoding: Set by user.
     * - decoding: unused
     */
    int rc_buffer_size;

    /**
     * ratecontrol override, see RcOverride
     * - encoding: Allocated/set/freed by user.
     * - decoding: unused
     */
    int rc_override_count;
    RcOverride *rc_override;

    /**
     * maximum bitrate
     * - encoding: Set by user.
     * - decoding: Set by user, may be overwritten by libavcodec.
     */
    int64_t rc_max_rate;

    /**
     * minimum bitrate
     * - encoding: Set by user.
     * - decoding: unused
     */
    int64_t rc_min_rate;

    /**
     * Ratecontrol attempt to use, at maximum, <value> of what can be used without an underflow.
     * - encoding: Set by user.
     * - decoding: unused.
     */
    float rc_max_available_vbv_use;

    /**
     * Ratecontrol attempt to use, at least, <value> times the amount needed to prevent a vbv overflow.
     * - encoding: Set by user.
     * - decoding: unused.
     */
    float rc_min_vbv_overflow_use;

    /**
     * Number of bits which should be loaded into the rc buffer before decoding starts.
     * - encoding: Set by user.
     * - decoding: unused
     */
    int rc_initial_buffer_occupancy;

    /**
     * trellis RD quantization
     * - encoding: Set by user.
     * - decoding: unused
     */
    int trellis;

    /**
     * pass1 encoding statistics output buffer
     * - encoding: Set by libavcodec.
     * - decoding: unused
     */
    char *stats_out;

    /**
     * pass2 encoding statistics input buffer
     * Concatenated stuff from stats_out of pass1 should be placed here.
     * - encoding: Allocated/set/freed by user.
     * - decoding: unused
     */
    char *stats_in;

    /**
     * Work around bugs in encoders which sometimes cannot be detected automatically.
     * - encoding: Set by user
     * - decoding: Set by user
     */
    int workaround_bugs;
#define FF_BUG_AUTODETECT       1  ///< autodetection
#define FF_BUG_XVID_ILACE       4
#define FF_BUG_UMP4             8
#define FF_BUG_NO_PADDING       16
#define FF_BUG_AMV              32
#define FF_BUG_QPEL_CHROMA      64
#define FF_BUG_STD_QPEL         128
#define FF_BUG_QPEL_CHROMA2     256
#define FF_BUG_DIRECT_BLOCKSIZE 512
#define FF_BUG_EDGE             1024
#define FF_BUG_HPEL_CHROMA      2048
#define FF_BUG_DC_CLIP          4096
#define FF_BUG_MS               8192 ///< Work around various bugs in Microsoft's broken decoders.
#define FF_BUG_TRUNCATED       16384
#define FF_BUG_IEDGE           32768

    /**
     * strictly follow the standard (MPEG-4, ...).
     * - encoding: Set by user.
     * - decoding: Set by user.
     * Setting this to STRICT or higher means the encoder and decoder will
     * generally do stupid things, whereas setting it to unofficial or lower
     * will mean the encoder might produce output that is not supported by all
     * spec-compliant decoders. Decoders don't differentiate between normal,
     * unofficial and experimental (that is, they always try to decode things
     * when they can) unless they are explicitly asked to behave stupidly
     * (=strictly conform to the specs)
     */
    int strict_std_compliance;
#define FF_COMPLIANCE_VERY_STRICT   2 ///< Strictly conform to an older more strict version of the spec or reference software.
#define FF_COMPLIANCE_STRICT        1 ///< Strictly conform to all the things in the spec no matter what consequences.
#define FF_COMPLIANCE_NORMAL        0
#define FF_COMPLIANCE_UNOFFICIAL   -1 ///< Allow unofficial extensions
#define FF_COMPLIANCE_EXPERIMENTAL -2 ///< Allow nonstandardized experimental things.

    /**
     * error concealment flags
     * - encoding: unused
     * - decoding: Set by user.
     */
    int error_concealment;
#define FF_EC_GUESS_MVS   1
#define FF_EC_DEBLOCK     2
#define FF_EC_FAVOR_INTER 256

    /**
     * debug
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int debug;
#define FF_DEBUG_PICT_INFO   1
#define FF_DEBUG_RC          2
#define FF_DEBUG_BITSTREAM   4
#define FF_DEBUG_MB_TYPE     8
#define FF_DEBUG_QP          16
#define FF_DEBUG_DCT_COEFF   0x00000040
#define FF_DEBUG_SKIP        0x00000080
#define FF_DEBUG_STARTCODE   0x00000100
#define FF_DEBUG_ER          0x00000400
#define FF_DEBUG_MMCO        0x00000800
#define FF_DEBUG_BUGS        0x00001000
#define FF_DEBUG_BUFFERS     0x00008000
#define FF_DEBUG_THREADS     0x00010000
#define FF_DEBUG_GREEN_MD    0x00800000
#define FF_DEBUG_NOMC        0x01000000

    /**
     * Error recognition; may misdetect some more or less valid parts as errors.
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int err_recognition;

/**
 * Verify checksums embedded in the bitstream (could be of either encoded or
 * decoded data, depending on the codec) and print an error message on mismatch.
 * If AV_EF_EXPLODE is also set, a mismatching checksum will result in the
 * decoder returning an error.
 */
#define AV_EF_CRCCHECK  (1<<0)
#define AV_EF_BITSTREAM (1<<1)          ///< detect bitstream specification deviations
#define AV_EF_BUFFER    (1<<2)          ///< detect improper bitstream length
#define AV_EF_EXPLODE   (1<<3)          ///< abort decoding on minor error detection

#define AV_EF_IGNORE_ERR (1<<15)        ///< ignore errors and continue
#define AV_EF_CAREFUL    (1<<16)        ///< consider things that violate the spec, are fast to calculate and have not been seen in the wild as errors
#define AV_EF_COMPLIANT  (1<<17)        ///< consider all spec non compliances as errors
#define AV_EF_AGGRESSIVE (1<<18)        ///< consider things that a sane encoder should not do as an error


    /**
     * opaque 64-bit number (generally a PTS) that will be reordered and
     * output in AVFrame.reordered_opaque
     * - encoding: Set by libavcodec to the reordered_opaque of the input
     *             frame corresponding to the last returned packet. Only
     *             supported by encoders with the
     *             AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE capability.
     * - decoding: Set by user.
     */
    int64_t reordered_opaque;

    /**
     * Hardware accelerator in use
     * - encoding: unused.
     * - decoding: Set by libavcodec
     */
    const struct AVHWAccel *hwaccel;

    /**
     * Hardware accelerator context.
     * For some hardware accelerators, a global context needs to be
     * provided by the user. In that case, this holds display-dependent
     * data FFmpeg cannot instantiate itself. Please refer to the
     * FFmpeg HW accelerator documentation to know how to fill this.
     * - encoding: unused
     * - decoding: Set by user
     */
    void *hwaccel_context;

    /**
     * error
     * - encoding: Set by libavcodec if flags & AV_CODEC_FLAG_PSNR.
     * - decoding: unused
     */
    uint64_t error[AV_NUM_DATA_POINTERS];

    /**
     * DCT algorithm, see FF_DCT_* below
     * - encoding: Set by user.
     * - decoding: unused
     */
    int dct_algo;
#define FF_DCT_AUTO    0
#define FF_DCT_FASTINT 1
#define FF_DCT_INT     2
#define FF_DCT_MMX     3
#define FF_DCT_ALTIVEC 5
#define FF_DCT_FAAN    6

    /**
     * IDCT algorithm, see FF_IDCT_* below.
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int idct_algo;
#define FF_IDCT_AUTO          0
#define FF_IDCT_INT           1
#define FF_IDCT_SIMPLE        2
#define FF_IDCT_SIMPLEMMX     3
#define FF_IDCT_ARM           7
#define FF_IDCT_ALTIVEC       8
#define FF_IDCT_SIMPLEARM     10
#define FF_IDCT_XVID          14
#define FF_IDCT_SIMPLEARMV5TE 16
#define FF_IDCT_SIMPLEARMV6   17
#define FF_IDCT_FAAN          20
#define FF_IDCT_SIMPLENEON    22
#if FF_API_IDCT_NONE
// formerly used by xvmc
#define FF_IDCT_NONE          24
#endif
#define FF_IDCT_SIMPLEAUTO    128

    /**
     * bits per sample/pixel from the demuxer (needed for huffyuv).
     * - encoding: Set by libavcodec.
     * - decoding: Set by user.
     */
     int bits_per_coded_sample;

    /**
     * Bits per sample/pixel of internal libavcodec pixel/sample format.
     * - encoding: set by user.
     * - decoding: set by libavcodec.
     */
    int bits_per_raw_sample;

    /**
     * low resolution decoding, 1-> 1/2 size, 2->1/4 size
     * - encoding: unused
     * - decoding: Set by user.
     */
     int lowres;

    /**
     * thread count
     * is used to decide how many independent tasks should be passed to execute()
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    int thread_count;

    /**
     * Which multithreading methods to use.
     * Use of FF_THREAD_FRAME will increase decoding delay by one frame per thread,
     * so clients which cannot provide future frames should not use it.
     *
     * - encoding: Set by user, otherwise the default is used.
     * - decoding: Set by user, otherwise the default is used.
     */
    int thread_type;
#define FF_THREAD_FRAME   1 ///< Decode more than one frame at once
#define FF_THREAD_SLICE   2 ///< Decode more than one part of a single frame at once

    /**
     * Which multithreading methods are in use by the codec.
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    int active_thread_type;

#if FF_API_THREAD_SAFE_CALLBACKS
    /**
     * Set by the client if its custom get_buffer() callback can be called
     * synchronously from another thread, which allows faster multithreaded decoding.
     * draw_horiz_band() will be called from other threads regardless of this setting.
     * Ignored if the default get_buffer() is used.
     * - encoding: Set by user.
     * - decoding: Set by user.
     *
     * @deprecated the custom get_buffer2() callback should always be
     *   thread-safe. Thread-unsafe get_buffer2() implementations will be
     *   invalid starting with LIBAVCODEC_VERSION_MAJOR=60; in other words,
     *   libavcodec will behave as if this field was always set to 1.
     *   Callers that want to be forward compatible with future libavcodec
     *   versions should wrap access to this field in
     *     #if LIBAVCODEC_VERSION_MAJOR < 60
     */
    attribute_deprecated
    int thread_safe_callbacks;
#endif

    /**
     * The codec may call this to execute several independent things.
     * It will return only after finishing all tasks.
     * The user may replace this with some multithreaded implementation,
     * the default implementation will execute the parts serially.
     * @param count the number of things to execute
     * - encoding: Set by libavcodec, user can override.
     * - decoding: Set by libavcodec, user can override.
     */
    int (*execute)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);

    /**
     * The codec may call this to execute several independent things.
     * It will return only after finishing all tasks.
     * The user may replace this with some multithreaded implementation,
     * the default implementation will execute the parts serially.
     * @param c context passed also to func
     * @param count the number of things to execute
     * @param arg2 argument passed unchanged to func
     * @param ret return values of executed functions, must have space for "count" values. May be NULL.
     * @param func function that will be called count times, with jobnr from 0 to count-1.
     *             threadnr will be in the range 0 to c->thread_count-1 < MAX_THREADS and so that no
     *             two instances of func executing at the same time will have the same threadnr.
     * @return always 0 currently, but code should handle a future improvement where when any call to func
     *         returns < 0 no further calls to func may be done and < 0 is returned.
     * - encoding: Set by libavcodec, user can override.
     * - decoding: Set by libavcodec, user can override.
     */
    int (*execute2)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);

    /**
     * noise vs. sse weight for the nsse comparison function
     * - encoding: Set by user.
     * - decoding: unused
     */
     int nsse_weight;

    /**
     * profile
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
     int profile;
#define FF_PROFILE_UNKNOWN -99
#define FF_PROFILE_RESERVED -100

#define FF_PROFILE_AAC_MAIN 0
#define FF_PROFILE_AAC_LOW  1
#define FF_PROFILE_AAC_SSR  2
#define FF_PROFILE_AAC_LTP  3
#define FF_PROFILE_AAC_HE   4
#define FF_PROFILE_AAC_HE_V2 28
#define FF_PROFILE_AAC_LD   22
#define FF_PROFILE_AAC_ELD  38
#define FF_PROFILE_MPEG2_AAC_LOW 128
#define FF_PROFILE_MPEG2_AAC_HE  131

#define FF_PROFILE_DNXHD         0
#define FF_PROFILE_DNXHR_LB      1
#define FF_PROFILE_DNXHR_SQ      2
#define FF_PROFILE_DNXHR_HQ      3
#define FF_PROFILE_DNXHR_HQX     4
#define FF_PROFILE_DNXHR_444     5

#define FF_PROFILE_DTS         20
#define FF_PROFILE_DTS_ES      30
#define FF_PROFILE_DTS_96_24   40
#define FF_PROFILE_DTS_HD_HRA  50
#define FF_PROFILE_DTS_HD_MA   60
#define FF_PROFILE_DTS_EXPRESS 70

#define FF_PROFILE_MPEG2_422    0
#define FF_PROFILE_MPEG2_HIGH   1
#define FF_PROFILE_MPEG2_SS     2
#define FF_PROFILE_MPEG2_SNR_SCALABLE  3
#define FF_PROFILE_MPEG2_MAIN   4
#define FF_PROFILE_MPEG2_SIMPLE 5

#define FF_PROFILE_H264_CONSTRAINED  (1<<9)  // 8+1; constraint_set1_flag
#define FF_PROFILE_H264_INTRA        (1<<11) // 8+3; constraint_set3_flag

#define FF_PROFILE_H264_BASELINE             66
#define FF_PROFILE_H264_CONSTRAINED_BASELINE (66|FF_PROFILE_H264_CONSTRAINED)
#define FF_PROFILE_H264_MAIN                 77
#define FF_PROFILE_H264_EXTENDED             88
#define FF_PROFILE_H264_HIGH                 100
#define FF_PROFILE_H264_HIGH_10              110
#define FF_PROFILE_H264_HIGH_10_INTRA        (110|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_MULTIVIEW_HIGH       118
#define FF_PROFILE_H264_HIGH_422             122
#define FF_PROFILE_H264_HIGH_422_INTRA       (122|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_STEREO_HIGH          128
#define FF_PROFILE_H264_HIGH_444             144
#define FF_PROFILE_H264_HIGH_444_PREDICTIVE  244
#define FF_PROFILE_H264_HIGH_444_INTRA       (244|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_CAVLC_444            44

#define FF_PROFILE_VC1_SIMPLE   0
#define FF_PROFILE_VC1_MAIN     1
#define FF_PROFILE_VC1_COMPLEX  2
#define FF_PROFILE_VC1_ADVANCED 3

#define FF_PROFILE_MPEG4_SIMPLE                     0
#define FF_PROFILE_MPEG4_SIMPLE_SCALABLE            1
#define FF_PROFILE_MPEG4_CORE                       2
#define FF_PROFILE_MPEG4_MAIN                       3
#define FF_PROFILE_MPEG4_N_BIT                      4
#define FF_PROFILE_MPEG4_SCALABLE_TEXTURE           5
#define FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION      6
#define FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE     7
#define FF_PROFILE_MPEG4_HYBRID                     8
#define FF_PROFILE_MPEG4_ADVANCED_REAL_TIME         9
#define FF_PROFILE_MPEG4_CORE_SCALABLE             10
#define FF_PROFILE_MPEG4_ADVANCED_CODING           11
#define FF_PROFILE_MPEG4_ADVANCED_CORE             12
#define FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE 13
#define FF_PROFILE_MPEG4_SIMPLE_STUDIO             14
#define FF_PROFILE_MPEG4_ADVANCED_SIMPLE           15

#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_0   1
#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_1   2
#define FF_PROFILE_JPEG2000_CSTREAM_NO_RESTRICTION  32768
#define FF_PROFILE_JPEG2000_DCINEMA_2K              3
#define FF_PROFILE_JPEG2000_DCINEMA_4K              4

#define FF_PROFILE_VP9_0                            0
#define FF_PROFILE_VP9_1                            1
#define FF_PROFILE_VP9_2                            2
#define FF_PROFILE_VP9_3                            3

#define FF_PROFILE_HEVC_MAIN                        1
#define FF_PROFILE_HEVC_MAIN_10                     2
#define FF_PROFILE_HEVC_MAIN_STILL_PICTURE          3
#define FF_PROFILE_HEVC_REXT                        4

#define FF_PROFILE_VVC_MAIN_10                      1
#define FF_PROFILE_VVC_MAIN_10_444                 33

#define FF_PROFILE_AV1_MAIN                         0
#define FF_PROFILE_AV1_HIGH                         1
#define FF_PROFILE_AV1_PROFESSIONAL                 2

#define FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT            0xc0
#define FF_PROFILE_MJPEG_HUFFMAN_EXTENDED_SEQUENTIAL_DCT 0xc1
#define FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT         0xc2
#define FF_PROFILE_MJPEG_HUFFMAN_LOSSLESS                0xc3
#define FF_PROFILE_MJPEG_JPEG_LS                         0xf7

#define FF_PROFILE_SBC_MSBC                         1

#define FF_PROFILE_PRORES_PROXY     0
#define FF_PROFILE_PRORES_LT        1
#define FF_PROFILE_PRORES_STANDARD  2
#define FF_PROFILE_PRORES_HQ        3
#define FF_PROFILE_PRORES_4444      4
#define FF_PROFILE_PRORES_XQ        5

#define FF_PROFILE_ARIB_PROFILE_A 0
#define FF_PROFILE_ARIB_PROFILE_C 1

#define FF_PROFILE_KLVA_SYNC 0
#define FF_PROFILE_KLVA_ASYNC 1

    /**
     * level
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
     int level;
#define FF_LEVEL_UNKNOWN -99

    /**
     * Skip loop filtering for selected frames.
     * - encoding: unused
     * - decoding: Set by user.
     */
    enum AVDiscard skip_loop_filter;

    /**
     * Skip IDCT/dequantization for selected frames.
     * - encoding: unused
     * - decoding: Set by user.
     */
    enum AVDiscard skip_idct;

    /**
     * Skip decoding for selected frames.
     * - encoding: unused
     * - decoding: Set by user.
     */
    enum AVDiscard skip_frame;

    /**
     * Header containing style information for text subtitles.
     * For SUBTITLE_ASS subtitle type, it should contain the whole ASS
     * [Script Info] and [V4+ Styles] section, plus the [Events] line and
     * the Format line following. It shouldn't include any Dialogue line.
     * - encoding: Set/allocated/freed by user (before avcodec_open2())
     * - decoding: Set/allocated/freed by libavcodec (by avcodec_open2())
     */
    uint8_t *subtitle_header;
    int subtitle_header_size;

    /**
     * Audio only. The number of "priming" samples (padding) inserted by the
     * encoder at the beginning of the audio. I.e. this number of leading
     * decoded samples must be discarded by the caller to get the original audio
     * without leading padding.
     *
     * - decoding: unused
     * - encoding: Set by libavcodec. The timestamps on the output packets are
     *             adjusted by the encoder so that they always refer to the
     *             first sample of the data actually contained in the packet,
     *             including any added padding.  E.g. if the timebase is
     *             1/samplerate and the timestamp of the first input sample is
     *             0, the timestamp of the first output packet will be
     *             -initial_padding.
     */
    int initial_padding;

    /**
     * - decoding: For codecs that store a framerate value in the compressed
     *             bitstream, the decoder may export it here. { 0, 1} when
     *             unknown.
     * - encoding: May be used to signal the framerate of CFR content to an
     *             encoder.
     */
    AVRational framerate;

    /**
     * Nominal unaccelerated pixel format, see AV_PIX_FMT_xxx.
     * - encoding: unused.
     * - decoding: Set by libavcodec before calling get_format()
     */
    enum AVPixelFormat sw_pix_fmt;

    /**
     * Timebase in which pkt_dts/pts and AVPacket.dts/pts are.
     * - encoding unused.
     * - decoding set by user.
     */
    AVRational pkt_timebase;

    /**
     * AVCodecDescriptor
     * - encoding: unused.
     * - decoding: set by libavcodec.
     */
    const AVCodecDescriptor *codec_descriptor;

    /**
     * Current statistics for PTS correction.
     * - decoding: maintained and used by libavcodec, not intended to be used by user apps
     * - encoding: unused
     */
    int64_t pts_correction_num_faulty_pts; /// Number of incorrect PTS values so far
    int64_t pts_correction_num_faulty_dts; /// Number of incorrect DTS values so far
    int64_t pts_correction_last_pts;       /// PTS of the last frame
    int64_t pts_correction_last_dts;       /// DTS of the last frame

    /**
     * Character encoding of the input subtitles file.
     * - decoding: set by user
     * - encoding: unused
     */
    char *sub_charenc;

    /**
     * Subtitles character encoding mode. Formats or codecs might be adjusting
     * this setting (if they are doing the conversion themselves for instance).
     * - decoding: set by libavcodec
     * - encoding: unused
     */
    int sub_charenc_mode;
#define FF_SUB_CHARENC_MODE_DO_NOTHING  -1  ///< do nothing (demuxer outputs a stream supposed to be already in UTF-8, or the codec is bitmap for instance)
#define FF_SUB_CHARENC_MODE_AUTOMATIC    0  ///< libavcodec will select the mode itself
#define FF_SUB_CHARENC_MODE_PRE_DECODER  1  ///< the AVPacket data needs to be recoded to UTF-8 before being fed to the decoder, requires iconv
#define FF_SUB_CHARENC_MODE_IGNORE       2  ///< neither convert the subtitles, nor check them for valid UTF-8

    /**
     * Skip processing alpha if supported by codec.
     * Note that if the format uses pre-multiplied alpha (common with VP6,
     * and recommended due to better video quality/compression)
     * the image will look as if alpha-blended onto a black background.
     * However for formats that do not use pre-multiplied alpha
     * there might be serious artefacts (though e.g. libswscale currently
     * assumes pre-multiplied alpha anyway).
     *
     * - decoding: set by user
     * - encoding: unused
     */
    int skip_alpha;

    /**
     * Number of samples to skip after a discontinuity
     * - decoding: unused
     * - encoding: set by libavcodec
     */
    int seek_preroll;

#if FF_API_DEBUG_MV
    /**
     * @deprecated unused
     */
    attribute_deprecated
    int debug_mv;
#define FF_DEBUG_VIS_MV_P_FOR  0x00000001 //visualize forward predicted MVs of P frames
#define FF_DEBUG_VIS_MV_B_FOR  0x00000002 //visualize forward predicted MVs of B frames
#define FF_DEBUG_VIS_MV_B_BACK 0x00000004 //visualize backward predicted MVs of B frames
#endif

    /**
     * custom intra quantization matrix
     * - encoding: Set by user, can be NULL.
     * - decoding: unused.
     */
    uint16_t *chroma_intra_matrix;

    /**
     * dump format separator.
     * can be ", " or "\n      " or anything else
     * - encoding: Set by user.
     * - decoding: Set by user.
     */
    uint8_t *dump_separator;

    /**
     * ',' separated list of allowed decoders.
     * If NULL then all are allowed
     * - encoding: unused
     * - decoding: set by user
     */
    char *codec_whitelist;

    /**
     * Properties of the stream that gets decoded
     * - encoding: unused
     * - decoding: set by libavcodec
     */
    unsigned properties;
#define FF_CODEC_PROPERTY_LOSSLESS        0x00000001
#define FF_CODEC_PROPERTY_CLOSED_CAPTIONS 0x00000002
#define FF_CODEC_PROPERTY_FILM_GRAIN      0x00000004

    /**
     * Additional data associated with the entire coded stream.
     *
     * - decoding: unused
     * - encoding: may be set by libavcodec after avcodec_open2().
     */
    AVPacketSideData *coded_side_data;
    int            nb_coded_side_data;

    /**
     * A reference to the AVHWFramesContext describing the input (for encoding)
     * or output (decoding) frames. The reference is set by the caller and
     * afterwards owned (and freed) by libavcodec - it should never be read by
     * the caller after being set.
     *
     * - decoding: This field should be set by the caller from the get_format()
     *             callback. The previous reference (if any) will always be
     *             unreffed by libavcodec before the get_format() call.
     *
     *             If the default get_buffer2() is used with a hwaccel pixel
     *             format, then this AVHWFramesContext will be used for
     *             allocating the frame buffers.
     *
     * - encoding: For hardware encoders configured to use a hwaccel pixel
     *             format, this field should be set by the caller to a reference
     *             to the AVHWFramesContext describing input frames.
     *             AVHWFramesContext.format must be equal to
     *             AVCodecContext.pix_fmt.
     *
     *             This field should be set before avcodec_open2() is called.
     */
    AVBufferRef *hw_frames_ctx;

#if FF_API_SUB_TEXT_FORMAT
    /**
     * @deprecated unused
     */
    attribute_deprecated
    int sub_text_format;
#define FF_SUB_TEXT_FMT_ASS              0
#endif

    /**
     * Audio only. The amount of padding (in samples) appended by the encoder to
     * the end of the audio. I.e. this number of decoded samples must be
     * discarded by the caller from the end of the stream to get the original
     * audio without any trailing padding.
     *
     * - decoding: unused
     * - encoding: unused
     */
    int trailing_padding;

    /**
     * The number of pixels per image to maximally accept.
     *
     * - decoding: set by user
     * - encoding: set by user
     */
    int64_t max_pixels;

    /**
     * A reference to the AVHWDeviceContext describing the device which will
     * be used by a hardware encoder/decoder.  The reference is set by the
     * caller and afterwards owned (and freed) by libavcodec.
     *
     * This should be used if either the codec device does not require
     * hardware frames or any that are used are to be allocated internally by
     * libavcodec.  If the user wishes to supply any of the frames used as
     * encoder input or decoder output then hw_frames_ctx should be used
     * instead.  When hw_frames_ctx is set in get_format() for a decoder, this
     * field will be ignored while decoding the associated stream segment, but
     * may again be used on a following one after another get_format() call.
     *
     * For both encoders and decoders this field should be set before
     * avcodec_open2() is called and must not be written to thereafter.
     *
     * Note that some decoders may require this field to be set initially in
     * order to support hw_frames_ctx at all - in that case, all frames
     * contexts used must be created on the same device.
     */
    AVBufferRef *hw_device_ctx;

    /**
     * Bit set of AV_HWACCEL_FLAG_* flags, which affect hardware accelerated
     * decoding (if active).
     * - encoding: unused
     * - decoding: Set by user (either before avcodec_open2(), or in the
     *             AVCodecContext.get_format callback)
     */
    int hwaccel_flags;

    /**
     * Video decoding only. Certain video codecs support cropping, meaning that
     * only a sub-rectangle of the decoded frame is intended for display.  This
     * option controls how cropping is handled by libavcodec.
     *
     * When set to 1 (the default), libavcodec will apply cropping internally.
     * I.e. it will modify the output frame width/height fields and offset the
     * data pointers (only by as much as possible while preserving alignment, or
     * by the full amount if the AV_CODEC_FLAG_UNALIGNED flag is set) so that
     * the frames output by the decoder refer only to the cropped area. The
     * crop_* fields of the output frames will be zero.
     *
     * When set to 0, the width/height fields of the output frames will be set
     * to the coded dimensions and the crop_* fields will describe the cropping
     * rectangle. Applying the cropping is left to the caller.
     *
     * @warning When hardware acceleration with opaque output frames is used,
     * libavcodec is unable to apply cropping from the top/left border.
     *
     * @note when this option is set to zero, the width/height fields of the
     * AVCodecContext and output AVFrames have different meanings. The codec
     * context fields store display dimensions (with the coded dimensions in
     * coded_width/height), while the frame fields store the coded dimensions
     * (with the display dimensions being determined by the crop_* fields).
     */
    int apply_cropping;

    /*
     * Video decoding only.  Sets the number of extra hardware frames which
     * the decoder will allocate for use by the caller.  This must be set
     * before avcodec_open2() is called.
     *
     * Some hardware decoders require all frames that they will use for
     * output to be defined in advance before decoding starts.  For such
     * decoders, the hardware frame pool must therefore be of a fixed size.
     * The extra frames set here are on top of any number that the decoder
     * needs internally in order to operate normally (for example, frames
     * used as reference pictures).
     */
    int extra_hw_frames;

    /**
     * The percentage of damaged samples to discard a frame.
     *
     * - decoding: set by user
     * - encoding: unused
     */
    int discard_damaged_percentage;

    /**
     * The number of samples per frame to maximally accept.
     *
     * - decoding: set by user
     * - encoding: set by user
     */
    int64_t max_samples;

    /**
     * Bit set of AV_CODEC_EXPORT_DATA_* flags, which affects the kind of
     * metadata exported in frame, packet, or coded stream side data by
     * decoders and encoders.
     *
     * - decoding: set by user
     * - encoding: set by user
     */
    int export_side_data;

    /**
     * This callback is called at the beginning of each packet to get a data
     * buffer for it.
     *
     * The following field will be set in the packet before this callback is
     * called:
     * - size
     * This callback must use the above value to calculate the required buffer size,
     * which must padded by at least AV_INPUT_BUFFER_PADDING_SIZE bytes.
     *
     * In some specific cases, the encoder may not use the entire buffer allocated by this
     * callback. This will be reflected in the size value in the packet once returned by
     * avcodec_receive_packet().
     *
     * This callback must fill the following fields in the packet:
     * - data: alignment requirements for AVPacket apply, if any. Some architectures and
     *   encoders may benefit from having aligned data.
     * - buf: must contain a pointer to an AVBufferRef structure. The packet's
     *   data pointer must be contained in it. See: av_buffer_create(), av_buffer_alloc(),
     *   and av_buffer_ref().
     *
     * If AV_CODEC_CAP_DR1 is not set then get_encode_buffer() must call
     * avcodec_default_get_encode_buffer() instead of providing a buffer allocated by
     * some other means.
     *
     * The flags field may contain a combination of AV_GET_ENCODE_BUFFER_FLAG_ flags.
     * They may be used for example to hint what use the buffer may get after being
     * created.
     * Implementations of this callback may ignore flags they don't understand.
     * If AV_GET_ENCODE_BUFFER_FLAG_REF is set in flags then the packet may be reused
     * (read and/or written to if it is writable) later by libavcodec.
     *
     * This callback must be thread-safe, as when frame threading is used, it may
     * be called from multiple threads simultaneously.
     *
     * @see avcodec_default_get_encode_buffer()
     *
     * - encoding: Set by libavcodec, user can override.
     * - decoding: unused
     */
    int (*get_encode_buffer)(struct AVCodecContext *s, AVPacket *pkt, int flags);

    /**
     * Audio channel layout.
     * - encoding: must be set by the caller, to one of AVCodec.ch_layouts.
     * - decoding: may be set by the caller if known e.g. from the container.
     *             The decoder can then override during decoding as needed.
     */
    AVChannelLayout ch_layout;
} AVCodecContext;

// AVCodec
//描述一个音频或视频编解码器
/**
 * AVCodec.
 */
typedef struct AVCodec {
    /**
     * Name of the codec implementation.
     * The name is globally unique among encoders and among decoders (but an
     * encoder and a decoder can share the same name).
     * This is the primary way to find a codec from the user perspective.
     */
    //编解码器的名称，例如 "h264"、"aac" 等
    const char *name;
    /**
     * Descriptive name for the codec, meant to be more human readable than name.
     * You should use the NULL_IF_CONFIG_SMALL() macro to define it.
     */
    // 编解码器的长名称，通常是更详细的描述，例如 "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"
    const char *long_name;
    // 编解码器的媒体类型，例如音频、视频或字幕
    enum AVMediaType type;
    // 编解码器的 ID，用于标识特定的编解码器
    enum AVCodecID id;
    /**
     * Codec capabilities.
     * see AV_CODEC_CAP_*
     */
    //编解码器的功能，例如是否支持多线程、是否支持硬件加速等
    int capabilities;
    uint8_t max_lowres;                     ///< maximum value for lowres supported by the decoder
    const AVRational *supported_framerates; ///< array of supported framerates, or NULL if any, array is terminated by {0,0}
    const enum AVPixelFormat *pix_fmts;     ///< array of supported pixel formats, or NULL if unknown, array is terminated by -1
    const int *supported_samplerates;       ///< array of supported audio samplerates, or NULL if unknown, array is terminated by 0
    const enum AVSampleFormat *sample_fmts; ///< array of supported sample formats, or NULL if unknown, array is terminated by -1
#if FF_API_OLD_CHANNEL_LAYOUT
    /**
     * @deprecated use ch_layouts instead
     */
    attribute_deprecated
    const uint64_t *channel_layouts;         ///< array of support channel layouts, or NULL if unknown. array is terminated by 0
#endif
    const AVClass *priv_class;              ///< AVClass for the private context
    const AVProfile *profiles;              ///< array of recognized profiles, or NULL if unknown, array is terminated by {FF_PROFILE_UNKNOWN}

    /**
     * Group name of the codec implementation.
     * This is a short symbolic name of the wrapper backing this codec. A
     * wrapper uses some kind of external implementation for the codec, such
     * as an external library, or a codec implementation provided by the OS or
     * the hardware.
     * If this field is NULL, this is a builtin, libavcodec native codec.
     * If non-NULL, this will be the suffix in AVCodec.name in most cases
     * (usually AVCodec.name will be of the form "<codec_name>_<wrapper_name>").
     */
    const char *wrapper_name;

    /**
     * Array of supported channel layouts, terminated with a zeroed layout.
     */
    const AVChannelLayout *ch_layouts;
} AVCodec;

// AVFrame
// 表示一个解码后的音频或视频帧
/**
 * This structure describes decoded (raw) audio or video data.
 *
 * AVFrame must be allocated using av_frame_alloc(). Note that this only
 * allocates the AVFrame itself, the buffers for the data must be managed
 * through other means (see below).
 * AVFrame must be freed with av_frame_free().
 *
 * AVFrame is typically allocated once and then reused multiple times to hold
 * different data (e.g. a single AVFrame to hold frames received from a
 * decoder). In such a case, av_frame_unref() will free any references held by
 * the frame and reset it to its original clean state before it
 * is reused again.
 *
 * The data described by an AVFrame is usually reference counted through the
 * AVBuffer API. The underlying buffer references are stored in AVFrame.buf /
 * AVFrame.extended_buf. An AVFrame is considered to be reference counted if at
 * least one reference is set, i.e. if AVFrame.buf[0] != NULL. In such a case,
 * every single data plane must be contained in one of the buffers in
 * AVFrame.buf or AVFrame.extended_buf.
 * There may be a single buffer for all the data, or one separate buffer for
 * each plane, or anything in between.
 *
 * sizeof(AVFrame) is not a part of the public ABI, so new fields may be added
 * to the end with a minor bump.
 *
 * Fields can be accessed through AVOptions, the name string used, matches the
 * C structure field name for fields accessible through AVOptions. The AVClass
 * for AVFrame can be obtained from avcodec_get_frame_class()
 */
typedef struct AVFrame {
#define AV_NUM_DATA_POINTERS 8
    /**
     * pointer to the picture/channel planes.
     * This might be different from the first allocated byte. For video,
     * it could even point to the end of the image data.
     *
     * All pointers in data and extended_data must point into one of the
     * AVBufferRef in buf or extended_buf.
     *
     * Some decoders access areas outside 0,0 - width,height, please
     * see avcodec_align_dimensions2(). Some filters and swscale can read
     * up to 16 bytes beyond the planes, if these filters are to be used,
     * then 16 extra bytes must be allocated.
     *
     * NOTE: Pointers not needed by the format MUST be set to NULL.
     *
     * @attention In case of video, the data[] pointers can point to the
     * end of image data in order to reverse line order, when used in
     * combination with negative values in the linesize[] array.
     */
    //指向帧数据缓冲区的指针数组。对于音频来说，data[0] 通常包含所有声道的数据；对于视频来说，data[0]、data[1] 和 data[2] 分别包含 Y、U 和 V 平面的数据
    uint8_t *data[AV_NUM_DATA_POINTERS];

    /**
     * For video, a positive or negative value, which is typically indicating
     * the size in bytes of each picture line, but it can also be:
     * - the negative byte size of lines for vertical flipping
     *   (with data[n] pointing to the end of the data
     * - a positive or negative multiple of the byte size as for accessing
     *   even and odd fields of a frame (possibly flipped)
     *
     * For audio, only linesize[0] may be set. For planar audio, each channel
     * plane must be the same size.
     *
     * For video the linesizes should be multiples of the CPUs alignment
     * preference, this is 16 or 32 for modern desktop CPUs.
     * Some code requires such alignment other code can be slower without
     * correct alignment, for yet other it makes no difference.
     *
     * @note The linesize may be larger than the size of usable data -- there
     * may be extra padding present for performance reasons.
     *
     * @attention In case of video, line size values can be negative to achieve
     * a vertically inverted iteration over image lines.
     */
    //每个数据平面的行大小。对于音频来说，linesize[0] 表示每个声道的样本数；对于视频来说，linesize[0]、linesize[1] 和 linesize[2] 分别表示 Y、U 和 V 平面的宽度
    int linesize[AV_NUM_DATA_POINTERS];

    /**
     * pointers to the data planes/channels.
     *
     * For video, this should simply point to data[].
     *
     * For planar audio, each channel has a separate data pointer, and
     * linesize[0] contains the size of each channel buffer.
     * For packed audio, there is just one data pointer, and linesize[0]
     * contains the total size of the buffer for all channels.
     *
     * Note: Both data and extended_data should always be set in a valid frame,
     * but for planar audio with more channels that can fit in data,
     * extended_data must be used in order to access all channels.
     */
    // 指向扩展数据缓冲区的指针，用于存储多于 AV_NUM_DATA_POINTERS 的数据平面
    uint8_t **extended_data;

    /**
     * @name Video dimensions
     * Video frames only. The coded dimensions (in pixels) of the video frame,
     * i.e. the size of the rectangle that contains some well-defined values.
     *
     * @note The part of the frame intended for display/presentation is further
     * restricted by the @ref cropping "Cropping rectangle".
     * @{
     */
    int width, height;
    /**
     * @}
     */

    /**
     * number of audio samples (per channel) described by this frame
     */
    //音频帧的采样数
    int nb_samples;

    /**
     * format of the frame, -1 if unknown or unset
     * Values correspond to enum AVPixelFormat for video frames,
     * enum AVSampleFormat for audio)
     */
    //帧的格式，对于音频来说，表示采样格式；对于视频来说，表示像素格式
    int format;

    /**
     * 1 -> keyframe, 0-> not
     */
    //是否为关键帧，如果为关键帧，则值为 1；否则为 0
    int key_frame;

    /**
     * Picture type of the frame.
     */
    //图片类型，例如 I 帧、P 帧或 B 帧
    enum AVPictureType pict_type;

    /**
     * Sample aspect ratio for the video frame, 0/1 if unknown/unspecified.
     */
    // 采样宽高比，用于表示视频帧的宽高比
    AVRational sample_aspect_ratio;

    /**
     * Presentation timestamp in time_base units (time when frame should be shown to user).
     */
    // 显示时间戳（PTS），用于表示帧应该何时显示
    int64_t pts;

    /**
     * DTS copied from the AVPacket that triggered returning this frame. (if frame threading isn't used)
     * This is also the Presentation time of this AVFrame calculated from
     * only AVPacket.dts values without pts values.
     */
    // 数据包中的解码时间戳（DTS）
    int64_t pkt_dts;

    /**
     * Time base for the timestamps in this frame.
     * In the future, this field may be set on frames output by decoders or
     * filters, but its value will be by default ignored on input to encoders
     * or filters.
     */
    AVRational time_base;

    /**
     * picture number in bitstream order
     */
    int coded_picture_number;
    /**
     * picture number in display order
     */
    int display_picture_number;

    /**
     * quality (between 1 (good) and FF_LAMBDA_MAX (bad))
     */
    int quality;

    /**
     * for some private data of the user
     */
    void *opaque;

    /**
     * When decoding, this signals how much the picture must be delayed.
     * extra_delay = repeat_pict / (2*fps)
     */
    int repeat_pict;

    /**
     * The content of the picture is interlaced.
     */
    int interlaced_frame;

    /**
     * If the content is interlaced, is top field displayed first.
     */
    int top_field_first;

    /**
     * Tell user application that palette has changed from previous frame.
     */
    int palette_has_changed;

    /**
     * reordered opaque 64 bits (generally an integer or a double precision float
     * PTS but can be anything).
     * The user sets AVCodecContext.reordered_opaque to represent the input at
     * that time,
     * the decoder reorders values as needed and sets AVFrame.reordered_opaque
     * to exactly one of the values provided by the user through AVCodecContext.reordered_opaque
     */
    int64_t reordered_opaque;

    /**
     * Sample rate of the audio data.
     */
    int sample_rate;

#if FF_API_OLD_CHANNEL_LAYOUT
    /**
     * Channel layout of the audio data.
     * @deprecated use ch_layout instead
     */
    attribute_deprecated
    uint64_t channel_layout;
#endif

    /**
     * AVBuffer references backing the data for this frame. All the pointers in
     * data and extended_data must point inside one of the buffers in buf or
     * extended_buf. This array must be filled contiguously -- if buf[i] is
     * non-NULL then buf[j] must also be non-NULL for all j < i.
     *
     * There may be at most one AVBuffer per data plane, so for video this array
     * always contains all the references. For planar audio with more than
     * AV_NUM_DATA_POINTERS channels, there may be more buffers than can fit in
     * this array. Then the extra AVBufferRef pointers are stored in the
     * extended_buf array.
     */
    AVBufferRef *buf[AV_NUM_DATA_POINTERS];

    /**
     * For planar audio which requires more than AV_NUM_DATA_POINTERS
     * AVBufferRef pointers, this array will hold all the references which
     * cannot fit into AVFrame.buf.
     *
     * Note that this is different from AVFrame.extended_data, which always
     * contains all the pointers. This array only contains the extra pointers,
     * which cannot fit into AVFrame.buf.
     *
     * This array is always allocated using av_malloc() by whoever constructs
     * the frame. It is freed in av_frame_unref().
     */
    AVBufferRef **extended_buf;
    /**
     * Number of elements in extended_buf.
     */
    int        nb_extended_buf;

    AVFrameSideData **side_data;
    int            nb_side_data;

/**
 * @defgroup lavu_frame_flags AV_FRAME_FLAGS
 * @ingroup lavu_frame
 * Flags describing additional frame properties.
 *
 * @{
 */

/**
 * The frame data may be corrupted, e.g. due to decoding errors.
 */
#define AV_FRAME_FLAG_CORRUPT       (1 << 0)
/**
 * A flag to mark the frames which need to be decoded, but shouldn't be output.
 */
#define AV_FRAME_FLAG_DISCARD   (1 << 2)
/**
 * @}
 */

    /**
     * Frame flags, a combination of @ref lavu_frame_flags
     */
    int flags;

    /**
     * MPEG vs JPEG YUV range.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorRange color_range;

    enum AVColorPrimaries color_primaries;

    enum AVColorTransferCharacteristic color_trc;

    /**
     * YUV colorspace type.
     * - encoding: Set by user
     * - decoding: Set by libavcodec
     */
    enum AVColorSpace colorspace;

    enum AVChromaLocation chroma_location;

    /**
     * frame timestamp estimated using various heuristics, in stream time base
     * - encoding: unused
     * - decoding: set by libavcodec, read by user.
     */
    int64_t best_effort_timestamp;

    /**
     * reordered pos from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64_t pkt_pos;

    /**
     * duration of the corresponding packet, expressed in
     * AVStream->time_base units, 0 if unknown.
     * - encoding: unused
     * - decoding: Read by user.
     */
    int64_t pkt_duration;

    /**
     * metadata.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    AVDictionary *metadata;

    /**
     * decode error flags of the frame, set to a combination of
     * FF_DECODE_ERROR_xxx flags if the decoder produced a frame, but there
     * were errors during the decoding.
     * - encoding: unused
     * - decoding: set by libavcodec, read by user.
     */
    int decode_error_flags;
#define FF_DECODE_ERROR_INVALID_BITSTREAM   1
#define FF_DECODE_ERROR_MISSING_REFERENCE   2
#define FF_DECODE_ERROR_CONCEALMENT_ACTIVE  4
#define FF_DECODE_ERROR_DECODE_SLICES       8

#if FF_API_OLD_CHANNEL_LAYOUT
    /**
     * number of audio channels, only used for audio.
     * - encoding: unused
     * - decoding: Read by user.
     * @deprecated use ch_layout instead
     */
    attribute_deprecated
    int channels;
#endif

    /**
     * size of the corresponding packet containing the compressed
     * frame.
     * It is set to a negative value if unknown.
     * - encoding: unused
     * - decoding: set by libavcodec, read by user.
     */
    int pkt_size;

    /**
     * For hwaccel-format frames, this should be a reference to the
     * AVHWFramesContext describing the frame.
     */
    AVBufferRef *hw_frames_ctx;

    /**
     * AVBufferRef for free use by the API user. FFmpeg will never check the
     * contents of the buffer ref. FFmpeg calls av_buffer_unref() on it when
     * the frame is unreferenced. av_frame_copy_props() calls create a new
     * reference with av_buffer_ref() for the target frame's opaque_ref field.
     *
     * This is unrelated to the opaque field, although it serves a similar
     * purpose.
     */
    AVBufferRef *opaque_ref;

    /**
     * @anchor cropping
     * @name Cropping
     * Video frames only. The number of pixels to discard from the the
     * top/bottom/left/right border of the frame to obtain the sub-rectangle of
     * the frame intended for presentation.
     * @{
     */
    size_t crop_top;
    size_t crop_bottom;
    size_t crop_left;
    size_t crop_right;
    /**
     * @}
     */

    /**
     * AVBufferRef for internal use by a single libav* library.
     * Must not be used to transfer data between libraries.
     * Has to be NULL when ownership of the frame leaves the respective library.
     *
     * Code outside the FFmpeg libs should never check or change the contents of the buffer ref.
     *
     * FFmpeg calls av_buffer_unref() on it when the frame is unreferenced.
     * av_frame_copy_props() calls create a new reference with av_buffer_ref()
     * for the target frame's private_ref field.
     */
    AVBufferRef *private_ref;

    /**
     * Channel layout of the audio data.
     */
    AVChannelLayout ch_layout;
} AVFrame;

#endif


}  // namespace cv
}  // namespace kingfisher

#endif
