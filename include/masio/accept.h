#ifndef __MASIO_ACCEPT_H__
#define __MASIO_ACCEPT_H__

namespace masio {

struct accept {
  using tcp = boost::asio::ip::tcp;
  using value_type = none_t;

  accept(tcp::socket& socket, unsigned short port)
    : socket(socket)
    , endpoint(tcp::endpoint(tcp::v4(), port))
  {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename Error<value_type>::Success;
    using Fail    = typename Error<value_type>::Fail;

    auto& ios = socket.get_io_service();

    if (canceler.canceled()) {
      ios.post([rest]() { rest(Fail{operation_aborted});});
      return;
    }

    auto acceptor = make_shared<tcp::acceptor>(ios, endpoint);

    auto cancel_action = make_shared<Canceler::CancelAction>([acceptor]() {
        acceptor->cancel();
        });

    canceler.link_cancel_action(*cancel_action);

    acceptor->async_accept(socket, [rest, &canceler, cancel_action, acceptor]
        (error_code error) {

        cancel_action->unlink();

        if (canceler.canceled()) {
          rest(Fail{operation_aborted});
        }
        else if (error) {
          rest(Fail{error});
        }
        else {
          rest(Success{none});
        }
        });
  }

  tcp::socket&  socket;
  tcp::endpoint endpoint;
};

} // masio namespace
#endif // ifndef __MASIO_ACCEPT_H__