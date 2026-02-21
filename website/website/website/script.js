// ===== SAMPLE FLIGHT DATA =====
const flights = [
    { id: "AI-101",  airline: "Air India",  from: "Delhi",     to: "Mumbai",     date: "2025-01-28", dep: "06:00", seats: 175, priceE: 4500,  priceB: 12000 },
    { id: "6E-205",  airline: "IndiGo",     from: "Delhi",     to: "Bangalore",  date: "2025-01-28", dep: "09:30", seats: 198, priceE: 5200,  priceB: 14000 },
    { id: "UK-833",  airline: "Vistara",    from: "Mumbai",    to: "Kolkata",    date: "2025-01-29", dep: "14:00", seats: 160, priceE: 4800,  priceB: 13000 },
    { id: "SG-422",  airline: "SpiceJet",   from: "Chennai",   to: "Delhi",      date: "2025-01-29", dep: "07:45", seats: 180, priceE: 3800,  priceB: 10000 },
    { id: "AI-305",  airline: "Air India",  from: "Kolkata",   to: "Hyderabad",  date: "2025-01-30", dep: "11:00", seats: 150, priceE: 4200,  priceB: 11500 },
    { id: "6E-117",  airline: "IndiGo",     from: "Bangalore", to: "Delhi",      date: "2025-01-30", dep: "16:30", seats: 200, priceE: 5500,  priceB: 15000 },
];

let bookings = [];
let bookingCounter = 9001;

// ===== RENDER FLIGHTS TABLE =====
function renderFlights(data) {
    const tbody = document.getElementById('flightBody');
    tbody.innerHTML = '';

    data.forEach(f => {
        tbody.innerHTML += `
            <tr>
                <td>${f.id}</td>
                <td>${f.airline}</td>
                <td>${f.from}</td>
                <td>${f.to}</td>
                <td>${f.date}</td>
                <td>${f.dep}</td>
                <td>${f.seats}</td>
                <td>₹${f.priceE}</td>
                <td><button class="book-btn" onclick="scrollToBooking('${f.id}')">Book</button></td>
            </tr>
        `;
    });
}

// ===== SEARCH FLIGHTS =====
function searchFlights() {
    const from = document.getElementById('searchFrom').value.toLowerCase();
    const to   = document.getElementById('searchTo').value.toLowerCase();
    const date = document.getElementById('searchDate').value;

    let filtered = flights.filter(f => {
        let match = true;
        if (from) match = match && f.from.toLowerCase().includes(from);
        if (to)   match = match && f.to.toLowerCase().includes(to);
        if (date) match = match && f.date === date;
        return match;
    });

    renderFlights(filtered);
}

// ===== SCROLL TO BOOKING =====
function scrollToBooking(flightId) {
    document.getElementById('booking').scrollIntoView({ behavior: 'smooth' });
    // Pre-select the flight
    document.getElementById('bFlight').value = flightId;
}

// ===== POPULATE FLIGHT DROPDOWN =====
function populateFlightDropdown() {
    const select = document.getElementById('bFlight');
    flights.forEach(f => {
        select.innerHTML += `<option value="${f.id}">${f.id} | ${f.from} → ${f.to} | ₹${f.priceE}</option>`;
    });
}

// ===== HANDLE BOOKING FORM =====
document.getElementById('bookingForm').addEventListener('submit', function(e) {
    e.preventDefault();

    const name     = document.getElementById('bName').value;
    const age      = document.getElementById('bAge').value;
    const gender   = document.getElementById('bGender').value;
    const phone    = document.getElementById('bPhone').value;
    const email    = document.getElementById('bEmail').value;
    const flightId = document.getElementById('bFlight').value;
    const seatCls  = document.getElementById('bClass').value;
    const passport = document.getElementById('bPassport').value;

    const flight = flights.find(f => f.id === flightId);
    const price  = seatCls === 'B' ? flight.priceB : flight.priceE;
    const seat   = Math.floor(Math.random() * 30 + 1) + String.fromCharCode(65 + Math.floor(Math.random() * 6));

    const booking = {
        bookingId: bookingCounter++,
        name, age, gender, phone, email, passport,
        flight: flight,
        seatClass: seatCls === 'B' ? 'Business' : 'Economy',
        seatNumber: seat,
        price: price,
        date: new Date().toLocaleDateString()
    };

    bookings.push(booking);
    showBoardingPass(booking);
    this.reset();
});

// ===== SHOW BOARDING PASS =====
function showBoardingPass(b) {
    const modal = document.getElementById('boardingPass');
    document.getElementById('passDetails').innerHTML = `
        <p><strong>Booking ID:</strong> ${b.bookingId}</p>
        <p><strong>Passenger:</strong> ${b.name}</p>
        <p><strong>Flight:</strong> ${b.flight.id} (${b.flight.airline})</p>
        <p><strong>Route:</strong> ${b.flight.from} → ${b.flight.to}</p>
        <p><strong>Date:</strong> ${b.flight.date}</p>
        <p><strong>Departure:</strong> ${b.flight.dep}</p>
        <p><strong>Seat:</strong> ${b.seatNumber} (${b.seatClass})</p>
        <p><strong>Amount:</strong> ₹${b.price}</p>
        <p><strong>Booked On:</strong> ${b.date}</p>
        <p style="color:green; font-weight:bold;">Status: CONFIRMED ✅</p>
    `;
    modal.style.display = 'flex';
}

function closeBoardingPass() {
    document.getElementById('boardingPass').style.display = 'none';
}

// ===== INITIALIZE =====
window.onload = function() {
    renderFlights(flights);
    populateFlightDropdown();
};
