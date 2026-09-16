#include "BufferBinaryReader.hpp"

#include <unistd.h>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <cerrno>
#include <iostream>

#include "BufferUtils.hpp"
#include "writer_config.hpp"
#include "buffer_config.hpp"

using namespace std;
using namespace writer_config;
using namespace buffer_config;

namespace {
    /** Reads up to n_bytes from the file at the given offset, dealing with
        partial reads and interrupted calls. Returns the number of read bytes
        (less than n_bytes on end of file), or -1 with errno set in case of
        error. */
    ssize_t pread_all(
            const int fd,
            void* buffer,
            const size_t n_bytes,
            const off_t offset)
    {
        auto data = static_cast<char*>(buffer);
        size_t n_read = 0;

        while (n_read < n_bytes) {
            auto n_current = ::pread(fd, data + n_read,
                                     n_bytes - n_read,
                                     offset + n_read);

            if (n_current < 0) {
                if (errno == EINTR) {
                    continue;
                }

                return -1;
            }

            // End of file.
            if (n_current == 0) {
                break;
            }

            n_read += static_cast<size_t>(n_current);
        }

        return static_cast<ssize_t>(n_read);
    }
}

BufferBinaryReader::BufferBinaryReader(
        const std::string &detector_folder,
        const std::string &module_name) :
        detector_folder_(detector_folder),
        module_name_(module_name),
        current_input_file_(""),
        input_file_fd_(-1)
{}

BufferBinaryReader::~BufferBinaryReader()
{
    close_current_file();
}

void BufferBinaryReader::get_block(
        const uint64_t block_id, BufferBinaryBlock* buffer)
{
    uint64_t block_start_pulse_id = block_id * BUFFER_BLOCK_SIZE;
    auto current_block_file = BufferUtils::get_filename(
            detector_folder_, module_name_, block_start_pulse_id);

    if (current_block_file != current_input_file_)  {
        open_file(current_block_file);
    }

    size_t file_start_index =
            BufferUtils::get_file_frame_index(block_start_pulse_id);
    size_t n_bytes_offset = file_start_index * sizeof(BufferBinaryFormat);

    auto n_bytes = pread_all(input_file_fd_, buffer,
            sizeof(BufferBinaryFormat) * BUFFER_BLOCK_SIZE, n_bytes_offset);

    if (n_bytes < static_cast<ssize_t>(sizeof(BufferBinaryFormat))) {
        stringstream err_msg;

        err_msg << "[BufferBinaryReader::get_block]";
        err_msg << " Error while reading from file ";
        err_msg << current_input_file_ << " for n_bytes_offset ";
        err_msg << n_bytes_offset << ": " << strerror(errno) << endl;

        cerr << err_msg.str();

        throw runtime_error(err_msg.str());
    }
}

void BufferBinaryReader::open_file(const std::string& filename)
{
    close_current_file();

    input_file_fd_ = open(filename.c_str(), O_RDONLY);

    if (input_file_fd_ < 0) {
        stringstream err_msg;

        err_msg << "[BufferBinaryReader::open_file]";
        err_msg << " Cannot open file " << filename << ": ";
        err_msg << strerror(errno) << endl;

        cerr << err_msg.str();

        throw runtime_error(err_msg.str());
    }

    current_input_file_ = filename;
}

void BufferBinaryReader::close_current_file()
{
    if (input_file_fd_ != -1) {
        if (close(input_file_fd_) < 0) {
            stringstream err_msg;

            err_msg << "[BufferBinaryReader::close_current_file]";
            err_msg << " Error while closing file " << current_input_file_;
            err_msg << ": " << strerror(errno) << endl;

            cerr << err_msg.str();

            throw runtime_error(err_msg.str());
        }

        input_file_fd_ = -1;
        current_input_file_ = "";
    }
}
