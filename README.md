***** CONTRIBUTOR *****
1. LINH
2. CHAU
3. VU
4. AN
5. DINH

Các bước fork() dự án và push request lên : 
 - sudo apt install git , git --version (nếu chưa cài đặt GIT)
 - Tải dự án (fork()) về máy tính cá nhân : git clone https://github.com/lamelihuynh/os-assigntment
 - Cấu hình user name và ID cho terminal , để máy tính biết bạn là ai 
           git config --global user.name <username của bạn trên github>
           git config --global user.mail <email mà bạn đăng kí trên github>
 - Tạo 1 nhánh mới và đặt tên cho nó , giả sử đang làm về phần scheduler thì nên đặt là branch-scheduler bằng lệnh : git checkout -b branch-scheduler, không nên thao tác trên nhánh chính main.
 - Kiểm tra bằng lệnh git branch xem đã tạo và ở branch mới chưa
 - Sau khi thao tác, lập trình, thay đổi trên các file thì bây giờ push lên lại dự án bằng cách 
           git add . 
           git commit -m "Mô tả thay đổi/ bạn đã thao tác , hiện thực trên những file nào ?"
           git push origin <tên branch> (lưu ý không phải main, vì lúc này đang ở branch khác)
- Lúc này phải xác thực bằng cách nhập user name và password cho hệ thống thì mới push lên được, username các bạn nhập như trên github đã đăng kí , còn password thì k phải nhập mật khẩu của tài khoản, các bạn vào trang này https://github.com/settings/tokens , chọn token(classic), generate token(classic) , nhập thông tin vào và chọn quyền repo, lưu ý phải là repo, sao đó generate token, lưu ý mã chỉ xem đc 1 lần nên các bạn nhớ lưu nó vô đâu đó sau đó dán vô phần password thì mới push lên được
- Sau đó chờ mình check , nếu thành công thì mình sẽ merge vào main cho các bạn , các bạn khác cũng có thể vào review cùng để BTL chúng ta tốt hơn vì nhiều người cùng check sẽ tốt hơn. 
