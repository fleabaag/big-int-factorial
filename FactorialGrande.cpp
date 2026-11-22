#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Representa un entero grande en base 1,000,000,000 (10^9) para reducir
// el número de operaciones y hacer el cálculo mucho más rápido.
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

    // Multiplica el número grande por un entero sin signo de 64 bits: *this *= n
    void multiply(uint64_t n) {
        if (n == 0 || (digits_.size() == 1 && digits_[0] == 0)) {
            digits_.assign(1, 0);
            return;
        }
        if (n == 1) {
            return;
        }

        uint64_t carry = 0;
        for (size_t i = 0; i < digits_.size(); ++i) {
            uint64_t prod = static_cast<uint64_t>(digits_[i]) * n + carry;
            digits_[i] = static_cast<uint32_t>(prod % BASE);
            carry = prod / BASE;
        }

        while (carry > 0) {
            digits_.push_back(static_cast<uint32_t>(carry % BASE));
            carry /= BASE;
        }
    }

    // Cantidad total de dígitos decimales
    size_t decimal_digits() const {
        if (digits_.empty()) return 0;
        size_t total = (digits_.size() - 1) * BASE_DIGITS;
        total += std::to_string(digits_.back()).size();
        return total;
    }

    // Convierte el número a su representación decimal en string
    std::string to_string() const {
        if (digits_.empty()) return "0";

        std::string result = std::to_string(digits_.back());
        for (size_t i = digits_.size() - 1; i > 0; --i) {
            std::string block = std::to_string(digits_[i - 1]);
            // Rellena con ceros a la izquierda para completar los 9 dígitos de la base
            result.append(BASE_DIGITS - block.size(), '0');
            result.append(block);
        }
        return result;
    }

private:
    std::vector<uint32_t> digits_; // De menos significativo (índice 0) a más significativo
};

BigInt factorial(uint64_t n) {
    BigInt result(1);
    for (uint64_t i = 2; i <= n; ++i) {
        result.multiply(i);
    }
    return result;
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
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "factorial_grande") << " <n>\n";
        std::cerr << "Ejemplo: " << (argc > 0 ? argv[0] : "factorial_grande") << " 1000\n";
        return 1;
    }

    try {
        uint64_t n = parse_non_negative_uint(argv[1]);

        auto start = std::chrono::steady_clock::now();
        BigInt result = factorial(n);
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double, std::milli> elapsed_ms = end - start;

        // Si el número es gigantesco (ej. > 100,000 dígitos), imprimirlo completo en consola
        // puede tardar más que el cálculo mismo; puedes ajustar o quitar este límite si lo deseas.
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