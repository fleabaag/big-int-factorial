#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class BigInt {
public:
    static constexpr uint32_t BASE = 1'000'000'000U;
    static constexpr int BASE_DIGITS = 9;

    explicit BigInt(uint64_t value = 0) {
        if (value == 0) {
            digits_.push_back(0);
        } else {
            while (value > 0) {
                digits_.push_back(static_cast<uint32_t>(value % BASE));
                value /= BASE;
            }
        }
    }

    bool is_zero() const {
        return digits_.size() == 1 && digits_[0] == 0;
    }

    // Multiplicación estándar entre dos BigInt en base 10^9: (*this) * other
    BigInt operator*(const BigInt& other) const {
        if (is_zero() || other.is_zero()) {
            return BigInt(0);
        }

        BigInt prod;
        prod.digits_.assign(digits_.size() + other.digits_.size(), 0);

        for (size_t i = 0; i < digits_.size(); ++i) {
            uint64_t a = digits_[i];
#if defined(__SIZEOF_INT128__)
            unsigned __int128 carry = 0;
            for (size_t j = 0; j < other.digits_.size(); ++j) {
                unsigned __int128 cur =
                    static_cast<unsigned __int128>(prod.digits_[i + j]) +
                    a * static_cast<unsigned __int128>(other.digits_[j]) +
                    carry;
                prod.digits_[i + j] = static_cast<uint32_t>(cur % BASE);
                carry = cur / BASE;
            }
            prod.digits_[i + other.digits_.size()] += static_cast<uint32_t>(carry);
#else
            uint64_t carry = 0;
            for (size_t j = 0; j < other.digits_.size(); ++j) {
                // Con BASE = 10^9, a * b < 10^18, y la suma máxima cabe en uint64_t
                uint64_t cur =
                    static_cast<uint64_t>(prod.digits_[i + j]) +
                    a * static_cast<uint64_t>(other.digits_[j]) +
                    carry;
                prod.digits_[i + j] = static_cast<uint32_t>(cur % BASE);
                carry = cur / BASE;
            }
            prod.digits_[i + other.digits_.size()] += static_cast<uint32_t>(carry);
#endif
        }

        while (prod.digits_.size() > 1 && prod.digits_.back() == 0) {
            prod.digits_.pop_back();
        }
        return prod;
    }

    size_t decimal_digits() const {
        if (digits_.empty()) return 0;
        size_t total = (digits_.size() - 1) * BASE_DIGITS;
        total += std::to_string(digits_.back()).size();
        return total;
    }

    std::string to_string() const {
        if (digits_.empty()) return "0";

        std::string result = std::to_string(digits_.back());
        for (size_t i = digits_.size() - 1; i > 0; --i) {
            std::string block = std::to_string(digits_[i - 1]);
            result.append(BASE_DIGITS - block.size(), '0');
            result.append(block);
        }
        return result;
    }

private:
    std::vector<uint32_t> digits_; // Menos significativo en índice 0
};

// Multiplica los enteros consecutivos en el rango inclusivo [l, r]
BigInt range_product(uint64_t l, uint64_t r) {
    if (l > r) {
        return BigInt(1);
    }
    if (l == r) {
        return BigInt(l);
    }
    // Caso base pequeño: acumular en uint64_t mientras no supere ~4 * 10^9
    if (r - l == 1) {
        // l * (l + 1) para n <= 10^7 cabe holgadamente en uint64_t (hasta ~10^14)
        return BigInt(l * r);
    }

    uint64_t mid = l + (r - l) / 2;
    BigInt left = range_product(l, mid);
    BigInt right = range_product(mid + 1, r);
    return left * right;
}

BigInt factorial_binary_splitting(uint64_t n) {
    if (n == 0 || n == 1) {
        return BigInt(1);
    }
    return range_product(2, n);
}

uint64_t parse_non_negative_uint(const std::string& arg) {
    if (arg.empty()) {
        throw std::invalid_argument("El argumento está vacío.");
    }
    for (char c : arg) {
        if (c < '0' || c > '9') {
            throw std::invalid_argument("El argumento debe ser un número entero no negativo.");
        }
    }
    try {
        size_t idx = 0;
        unsigned long long val = std::stoull(arg, &idx, 10);
        if (idx != arg.size()) {
            throw std::invalid_argument("El argumento contiene caracteres inválidos.");
        }
        return static_cast<uint64_t>(val);
    } catch (const std::out_of_range&) {
        throw std::out_of_range("El número de entrada excede el rango permitido.");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "factorial_binsplit") << " <n>\n";
        std::cerr << "Ejemplo: " << (argc > 0 ? argv[0] : "factorial_binsplit") << " 1000000\n";
        return 1;
    }

    try {
        uint64_t n = parse_non_negative_uint(argv[1]);

        auto start = std::chrono::steady_clock::now();
        BigInt result = factorial_binary_splitting(n);
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double, std::milli> elapsed_ms = end - start;

        std::string str_val = result.to_string();
        constexpr size_t MAX_PRINT_CHARS = 200'000;

        if (str_val.size() <= MAX_PRINT_CHARS) {
            std::cout << n << "! = " << str_val << "\n";
        } else {
            std::cout << n << "! (primeros 50 dígitos): " << str_val.substr(0, 50) << "...\n";
            std::cout << n << "! (últimos 50 dígitos): ..." << str_val.substr(str_val.size() - 50) << "\n";
        }

        std::cout << "Total de digitos: " << result.decimal_digits() << "\n";
        std::cout << "Tiempo de calculo: " << elapsed_ms.count() << " ms\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}