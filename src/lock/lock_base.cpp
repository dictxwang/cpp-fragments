
#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <shared_mutex>
#include <iostream>

using namespace std;

class LockComposite {

    public:
        LockComposite() {
            this->shared_rw_lock_ = make_shared<shared_mutex>();
        };
        ~LockComposite() {};
    private:
        shared_mutex rw_lock_; // Effective when used alone
        shared_ptr<shared_mutex> shared_rw_lock_; // Effective when used alone
        int counter;
    
    public:
        shared_mutex& get_lock() {
            return this->rw_lock_;
        }
        shared_ptr<shared_mutex> get_shared_lock() {
            return this->shared_rw_lock_;
        }

        int get_counter() {
            std::unique_lock<std::shared_mutex> w_lock(*this->shared_rw_lock_);
            this->counter += 1;
            return this->counter;
        }
};

int main(int argc, char const *argv[])
{

    shared_ptr<LockComposite> locker = make_shared<LockComposite>();

    thread t3([locker]{
        while (true) {
            int counter = locker->get_counter();
            cout << "t3 got counter: " << counter << endl;
            this_thread::sleep_for(chrono::seconds(1));
        }
    });
    t3.detach();

    thread t1([locker]{
        std::unique_lock<std::shared_mutex> w_lock(*locker->get_shared_lock());
        cout << "t1 got lock" << endl;
        this_thread::sleep_for(chrono::seconds(3));
        w_lock.unlock();
        cout << "t1 release lock" << endl;
    });
    t1.detach();
    thread t2([locker]{
        std::unique_lock<std::shared_mutex> w_lock(*locker->get_shared_lock());
        cout << "t2 got lock" << endl;
        this_thread::sleep_for(chrono::seconds(3));
        w_lock.unlock();
        cout << "t2 release lock" << endl;
    });
    t2.detach();

    while(true) {
        cout << "Keep Running..." << endl;
        std::this_thread::sleep_for(chrono::seconds(3));
    }

    /* code */
    return 0;
}
