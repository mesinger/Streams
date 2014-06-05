#ifndef STREAM_OPERATORS_IMPL_H
#define STREAM_OPERATORS_IMPL_H

template<typename T>
template<typename Predicate>
Stream<T> StreamImpl<T, Common>::filter(Predicate&& predicate) {
    check_vacant("filter");
    return make_stream_provider<FilteredStreamProvider, T, Predicate>
        (std::move(source_), std::forward<Predicate>(predicate));
}

template<typename T>
template<typename Predicate>
Stream<T> StreamImpl<T, Common>::take_while(Predicate&& predicate) {
    check_vacant("take_while");
    return make_stream_provider<TakeWhileStreamProvider, T, Predicate>
        (std::move(source_), std::forward<Predicate>(predicate));
}

template<typename T>
template<typename Predicate>
Stream<T> StreamImpl<T, Common>::drop_while(Predicate&& predicate) {
    check_vacant("drop_while");
    return make_stream_provider<DropWhileStreamProvider, T, Predicate>
        (std::move(source_), std::forward<Predicate>(predicate));
}

template<typename T>
Stream<T> StreamImpl<T, Common>::slice(size_t start, size_t end, size_t increment) {
    check_vacant("slice");
    return make_stream_provider<SlicedStreamProvider, T>
        (std::move(source_), start, end, increment, false);
}

template<typename T>
Stream<T> StreamImpl<T, Common>::slice_to_end(size_t start, size_t increment) {
    check_vacant("slice_to_end");
    return make_stream_provider<SlicedStreamProvider, T>
        (std::move(source_), start, 0, increment, true);
}

template<typename T>
template<typename Action>
Stream<T> StreamImpl<T, Common>::peek(Action&& action) {
    check_vacant("peek");
    return make_stream_provider<PeekStreamProvider, T, Action>
        (std::move(source_), std::forward<Action>(action));
}

template<typename T>
template<typename Transform>
Stream<ReturnType<Transform, T&&>> StreamImpl<T, Common>::map(Transform&& transform) {
    using Result = ReturnType<Transform, T&&>;
    static_assert(!std::is_void<Result>::value,
        "Return type of the mapping function cannot be void.");

    check_vacant("map");
    return make_stream_provider<MappedStreamProvider, Result, Transform, T>
        (std::move(source_), std::forward<Transform>(transform));
}

template<typename T>
template<typename Transform>
Stream<StreamType<ReturnType<Transform, T&&>>>
StreamImpl<T, Common>::flat_map(Transform&& transform) {
    using Result = ReturnType<Transform, T&&>;
    static_assert(IsStream<Result>::value,
        "Flat map must be passed a function which returns a stream");
    using S = StreamType<Result>;

    check_vacant("flat_map");
    return make_stream_provider<FlatMappedStreamProvider, S, Transform, T>
        (std::move(source_), std::forward<Transform>(transform));
}

template<typename T>
Stream<T> StreamImpl<T, Common>::limit(std::size_t length) {
    check_vacant("limit");
    return slice(0, length);
}

template<typename T>
Stream<T> StreamImpl<T, Common>::skip(std::size_t amount) {
    check_vacant("skip");
    return slice_to_end(amount, 1);
}

template<typename T>
template<typename Equal>
Stream<T> StreamImpl<T, Common>::adjacent_distinct(Equal&& equal) {
    check_vacant("adjacent_distinct");
    return make_stream_provider<AdjacentDistinctStreamProvider, T, Equal>
        (std::move(source_), std::forward<Equal>(equal));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::sort(Compare&& comparator) {
    check_vacant("sort");
    return make_stream_provider<SortedStreamProvider, T, Compare>
        (std::move(source_), std::forward<Compare>(comparator));
}

template<typename T>
Stream<T> StreamImpl<T, Common>::state_point() {
    check_vacant("state_point");
    return make_stream_provider<StatefulStreamProvider, T>
        (std::move(source_));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::distinct(Compare&& comparator) {
    check_vacant("distinct");
    return make_stream_provider<DistinctStreamProvider, T, Compare>
        (std::move(source_), std::forward<Compare>(comparator));
}

template<typename T>
template<typename Iterator>
Stream<T> StreamImpl<T, Common>::concat(Iterator begin, Iterator end) {
    return concat(Stream<T>(begin, end));
}

template<typename T>
Stream<T> StreamImpl<T, Common>::concat(Stream<T>&& other) {
    check_vacant("concat");
    auto concat_ptr = dynamic_cast<ConcatenatedStreamProvider<T>*>(source_.get());
    if(concat_ptr) {
        concat_ptr->concat(std::move(other.source_));
        return std::move(*this);
    }
    return make_stream_provider<ConcatenatedStreamProvider, T>
        (std::move(source_), std::move(other.source_));
}

template<typename T>
Stream<T> StreamImpl<T, Common>::pad(T&& padding) {
    return concat(MakeStream::repeat(std::forward<T>(padding)));
}

template<typename T>
Stream<GroupResult<T, 2>> StreamImpl<T, Common>::pairwise() {
    check_vacant("pairwise");
    return grouped<2>();
}

template<typename T>
template<size_t N>
Stream<GroupResult<T, N>> StreamImpl<T, Common>::grouped() {
    using GroupType = GroupResult<T, N>;

    check_vacant("grouped");
    return std::move(StreamProviderPtr<GroupType>(
        new GroupedStreamProvider<T, N>(std::move(source_))));
}

template<typename T>
template<typename Subtractor>
Stream<ReturnType<Subtractor, T&, T&>>
StreamImpl<T, Common>::adjacent_difference(Subtractor&& subtract) {
    using Result = ReturnType<Subtractor, T&, T&>;
    static_assert(!std::is_void<Result>::value,
        "Return type of the subtraction cannot be void.");

    check_vacant("adjacent_difference");
    return std::move(StreamProviderPtr<Result>(
        new AdjacentDifferenceStreamProvider<T, Subtractor>(
            std::move(source_), std::forward<Subtractor>(subtract))));
}

template<typename T>
template<typename Adder>
Stream<T> StreamImpl<T, Common>::partial_sum(Adder&& add) {
    check_vacant("partial_sum");
    return make_stream_provider<PartialSumStreamProvider, T, Adder>
        (std::move(source_), std::forward<Adder>(add));
}

template<typename T>
template<typename Other, typename Function>
Stream<ReturnType<Function, T&&, Other&&>> StreamImpl<T, Common>::zip_with(
        Stream<Other>&& other, Function&& zipper) {
    using Result = ReturnType<Function, T&&, Other&&>;
    static_assert(!std::is_void<Result>::value,
        "Return type of the zipping function cannot be void.");

    check_vacant("zip_with");
    return std::move(StreamProviderPtr<Result>(
        new ZippedStreamProvider<T, Other, Function>(
            std::move(source_), std::move(other.source_),
            std::forward<Function>(zipper))));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::merge_with(Stream<T>&& other, Compare&& compare) {
    check_vacant("merge_with");
    return make_stream_provider<MergedStreamProvider, T, Compare>
        (std::move(source_), std::move(other.source_),
         std::forward<Compare>(compare));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::union_with(Stream<T>&& other, Compare&& compare) {
    check_vacant("union_with");
    return make_stream_provider<UnionStreamProvider, T, Compare>
        (std::move(source_), std::move(other.source_),
         std::forward<Compare>(compare));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::intersection_with(Stream<T>&& other, Compare&& compare) {
    check_vacant("intersection_with");
    return make_stream_provider<IntersectionStreamProvider, T, Compare>
        (std::move(source_), std::move(other.source_),
         std::forward<Compare>(compare));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::difference_with(Stream<T>&& other, Compare&& compare) {
    check_vacant("difference_with");
    return make_stream_provider<DifferenceStreamProvider, T, Compare>
        (std::move(source_), std::move(other.source_),
         std::forward<Compare>(compare));
}

template<typename T>
template<typename Compare>
Stream<T> StreamImpl<T, Common>::symmetric_difference_with(Stream<T>&& other, Compare&& compare) {
    check_vacant("symmetric_difference_with");
    return make_stream_provider<SymmetricDifferenceStreamProvider, T, Compare>
        (std::move(source_), std::move(other.source_),
         std::forward<Compare>(compare));
}

#endif