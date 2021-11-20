import socket
import logging
from sys import stderr, argv
from typing import Optional, Iterator, Callable, Union


class Client:
    VT_ENCODING = 'utf-8'

    def __init__(self, host: str, port: int):
        self._host, self._port = host, port
        self._logger = logging.getLogger('db-client')
        self._s = None

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def connect(self) -> bool:
        try:
            self.close()
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self._host, self._port))
            self._s = sock
            return True
        except socket.timeout:
            self._logger.error('Socket connect timed out.')
        except InterruptedError:
            self._logger.error('Connection is aborted.')
        except IOError as e:
            self._logger.error(f'I/O error occurred while connecting: {e}')
        return False

    def close(self):
        if isinstance(self._s, socket.socket):
            self._s.close()
            self._s = None

    def _recv_cstr(self) -> bytes:
        sock = self._s
        assert isinstance(sock, socket.socket)
        s = b''
        while (c := sock.recv(1)) != b'\0':
            s += c
        return s

    def execute(self, command: str) -> Optional[str]:
        sock = self._s
        if not isinstance(sock, socket.socket):
            return
        sock.sendall(command.encode(self.VT_ENCODING))
        sock.sendall(b'\0')
        return self._recv_cstr().decode(self.VT_ENCODING)


class TestRunner:

    def __init__(self, client: Client, testcases: Iterator[str]):
        self._client, self._cases = client, testcases
        self._logger = logging.getLogger('test-runner')

    def run_test(self, callback: Callable[[str, str], Optional[bool]]):
        for i, testcase in enumerate(self._cases):
            if testcase.startswith('//'):
                continue
            self._logger.info(f'Run test {i + 1}.')
            response = self._client.execute(testcase)
            if callback(testcase, response) is False:
                break


class ReportingTester:
    """
    A test response handler which generates a test report.
    """
    _render_buf: str
    _test_counter: int

    def __init__(self):
        self.reset_context()

    def run_test(self, runner: TestRunner) -> str:
        runner.run_test(self._handler)
        s = self._render_buf
        self.reset_context()
        return s

    def reset_context(self):
        self._render_buf, self._test_counter = '', 0

    def _handler(self, testcase: str, response: str) -> Optional[bool]:
        self._test_counter += 1
        self._render_buf += f'{self._test_counter}. `{testcase}`\n```\n{response}\n```\n'
        return


if __name__ == '__main__':
    TEST_FILE_ENCODING = 'utf-8'
    REPORT_FILE_ENCODING = 'utf-8'
    host, port, test_file, report_file = None, None, None, None
    try:
        host, port, test_file, report_file = 'localhost', 6789, argv[1], None
        if '-o' in argv:
            i = argv.index('-o')
            report_file = argv[i + 1]
            argv.pop(i + 1)
            argv.pop(i)
        if len(argv) > 4 or len(argv) < 2:
            raise ValueError
        if len(argv) > 2:
            host = argv[2]
        if len(argv) > 3:
            port = argv[3]
    except (IndexError, ValueError):
        print('Usage: runtest.py <testfile> [host] [port] [-o <report_file>]')
        exit()

    print(f'Connecting to {host}:{port}...')
    with Client(host, port) as client, \
            open(test_file, 'r', encoding=TEST_FILE_ENCODING) as f:
        if not client.connect():
            print('Cannot connect to observer.', file=stderr)
            exit()
        tester = ReportingTester()
        test_report = tester.run_test(TestRunner(client, filter(lambda x: x.strip(), f)))
        if report_file:
            with open(report_file, 'w', encoding=REPORT_FILE_ENCODING) as of:
                of.write(test_report)
            print(f'Test report is saved in file {report_file}.')
        else:
            print(test_report)
