// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_QUIC_CORE_WEB_TRANSPORT_STREAM_ADAPTER_H_
#define QUICHE_QUIC_CORE_WEB_TRANSPORT_STREAM_ADAPTER_H_

#include "quic/core/quic_session.h"
#include "quic/core/quic_stream.h"
#include "quic/core/quic_stream_sequencer.h"
#include "quic/core/web_transport_interface.h"

namespace quic {

// Converts WebTransportStream API calls into QuicStream API calls.  The users
// of this class can either subclass it, or wrap around it.
class QUIC_EXPORT_PRIVATE WebTransportStreamAdapter
    : public WebTransportStream {
 public:
  WebTransportStreamAdapter(QuicSession* session,
                            QuicStream* stream,
                            QuicStreamSequencer* sequencer);

  // WebTransportStream implementation.
  size_t Read(char* buffer, size_t buffer_size) override;
  size_t Read(std::string* output) override;
  ABSL_MUST_USE_RESULT bool Write(absl::string_view data) override;
  ABSL_MUST_USE_RESULT bool SendFin() override;
  bool CanWrite() const override;
  size_t ReadableBytes() const override;
  void SetVisitor(std::unique_ptr<WebTransportStreamVisitor> visitor) override {
    visitor_ = std::move(visitor);
  }
  QuicStreamId GetStreamId() const override { return stream_->id(); }

  void ResetWithUserCode(QuicRstStreamErrorCode error) override {
    stream_->Reset(error);
  }
  void ResetDueToInternalError() override {
    stream_->Reset(QUIC_STREAM_INTERNAL_ERROR);
  }
  void MaybeResetDueToStreamObjectGone() override {
    if (stream_->write_side_closed() && stream_->read_side_closed()) {
      return;
    }
    stream_->Reset(QUIC_STREAM_CANCELLED);
  }

  WebTransportStreamVisitor* visitor() override { return visitor_.get(); }

  // Calls that need to be passed from the corresponding QuicStream methods.
  void OnDataAvailable();
  void OnCanWriteNewData();

 private:
  void MaybeNotifyFinRead();

  QuicSession* session_;            // Unowned.
  QuicStream* stream_;              // Unowned.
  QuicStreamSequencer* sequencer_;  // Unowned.
  std::unique_ptr<WebTransportStreamVisitor> visitor_;
  bool fin_read_notified_ = false;
};

}  // namespace quic

#endif  // QUICHE_QUIC_CORE_WEB_TRANSPORT_STREAM_ADAPTER_H_
