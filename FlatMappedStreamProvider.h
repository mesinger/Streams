#ifndef FLAT_MAPPED_STREAM_PROVIDER_H
#define FLAT_MAPPED_STREAM_PROVIDER_H

#include "StreamProvider.h"
#include "Stream.h"
#include "Utility.h"

template<typename T> struct Stream;

template<typename T>
struct StreamIdentifier {  using Type = void; };
template<typename T>
struct StreamIdentifier<Stream<T>> { using Type = T; };

template<typename S> using StreamType = typename StreamIdentifier<S>::Type;

template<typename T>
struct IsStream { enum { value = false }; };
template<typename T>
struct IsStream<Stream<T>> { enum { value = true }; };

template<typename T, typename Transform, typename In>
class FlatMappedStreamProvider : public StreamProvider<T> {

public:
    FlatMappedStreamProvider(StreamProviderPtr<In> source,
                             Transform&& transform)
        : source_(std::move(source)), transform_(transform) {}

    std::shared_ptr<T> get() override {
        return current_;
    }

    bool advance() override {
        if(!first_ && current_stream_.source_->advance()) {
            current_ = current_stream_.source_->get();
            return true;
        }

        if(first_)
            first_ = false;

        while(source_->advance()) {
            current_stream_ = std::move(transform_(*source_->get()));
            if(current_stream_.source_->advance()) {
                current_ = current_stream_.source_->get();
                return true;
            }
        }

        current_.reset();
        return false;
    }

private:
    StreamProviderPtr<In> source_;
    Transform transform_;
    Stream<T> current_stream_;
    std::shared_ptr<T> current_;
    bool first_ = true;

};

#endif