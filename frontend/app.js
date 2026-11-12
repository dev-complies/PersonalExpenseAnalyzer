/**
 * Personal Expense Analyzer & Smart Splitter Frontend Application
 */

// Configuration: API Base URL
const API_BASE_URL = window.location.origin.includes('localhost') || window.location.origin.includes('127.0.0.1')
  ? `${window.location.origin}/api`
  : 'http://localhost:8080/api';

// Application State
const state = {
  expenses: [],
  filteredExpenses: [],
  analytics: null,
  budgets: [],
  groups: [],
  activeGroupId: null,
  activeGroupDetails: null,
  users: [],
  currentTab: 'dashboard',
  filters: {
    search: '',
    category: 'ALL',
    sortBy: 'date-desc'
  },
  charts: {
    categoryDonut: null,
    monthlyTrend: null,
    dailyTimeline: null
  }
};

// ============================================================
// UTILITIES
// ============================================================

function formatRupees(paise) {
  if (paise === null || paise === undefined || isNaN(paise)) return '₹0.00';
  const rupees = paise / 100.0;
  return new Intl.NumberFormat('en-IN', {
    style: 'currency',
    currency: 'INR',
    minimumFractionDigits: 2,
    maximumFractionDigits: 2
  }).format(rupees);
}

function rupeesToPaise(rupees) {
  return Math.round(parseFloat(rupees) * 100);
}

function paiseToRupees(paise) {
  return (paise / 100.0).toFixed(2);
}

function escapeHtml(str) {
  if (!str) return '';
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

function getTodayString() {
  const now = new Date();
  const year = now.getFullYear();
  const month = String(now.getMonth() + 1).padStart(2, '0');
  const day = String(now.getDate()).padStart(2, '0');
  return `${year}-${month}-${day}`;
}

function showToast(message, type = 'success') {
  const container = document.getElementById('toastContainer');
  if (!container) return;

  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  toast.innerHTML = `<span>${escapeHtml(message)}</span>`;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(10px)';
    toast.style.transition = 'all 0.25s ease';
    setTimeout(() => toast.remove(), 250);
  }, 3200);
}

// ============================================================
// API CLIENT
// ============================================================

async function apiRequest(endpoint, options = {}) {
  const statusBadge = document.getElementById('apiStatusBadge');
  try {
    const url = `${API_BASE_URL}${endpoint}`;
    const res = await fetch(url, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...(options.headers || {})
      }
    });

    if (statusBadge) {
      statusBadge.classList.remove('offline');
      statusBadge.querySelector('.status-text').textContent = 'Connected';
    }

    const json = await res.json();
    if (!json.success) {
      throw new Error(json.message || 'API request returned an error');
    }
    return json.data;
  } catch (err) {
    if (statusBadge) {
      statusBadge.classList.add('offline');
      statusBadge.querySelector('.status-text').textContent = 'Offline';
    }
    console.error(`API Error on ${endpoint}:`, err);
    throw err;
  }
}

// ============================================================
// DATA FETCHING & SYNCHRONIZATION
// ============================================================

async function syncAllData() {
  try {
    const [expenses, analytics, budgets, groups, users] = await Promise.all([
      apiRequest('/expenses').catch(() => []),
      apiRequest('/analytics').catch(() => null),
      apiRequest('/budgets').catch(() => []),
      apiRequest('/groups').catch(() => []),
      apiRequest('/users').catch(() => [])
    ]);

    state.expenses = expenses || [];
    state.analytics = analytics;
    state.budgets = budgets || [];
    state.groups = groups || [];
    state.users = users || [];

    updateCategoryFilterDropdown();
    applyExpenseFilters();
    renderDashboard();
    renderAnalyticsTab();
    renderBudgetsTab();
    renderGroupsSidebar();

    if (state.groups.length > 0 && !state.activeGroupId) {
      selectGroup(state.groups[0].id);
    } else if (state.activeGroupId) {
      selectGroup(state.activeGroupId);
    }
  } catch (err) {
    showToast('Failed to load data from server. Please check if backend is running.', 'error');
  }
}

// ============================================================
// DASHBOARD RENDERING
// ============================================================

function renderDashboard() {
  const analytics = state.analytics || {
    totalExpenseCount: state.expenses.length,
    totalSpendingPaise: state.expenses.reduce((acc, e) => acc + (e.amountPaise || 0), 0),
    averageExpensePaise: state.expenses.length ? Math.round(state.expenses.reduce((acc, e) => acc + (e.amountPaise || 0), 0) / state.expenses.length) : 0,
    hasExpenses: state.expenses.length > 0
  };

  document.getElementById('dashTotalSpending').textContent = formatRupees(analytics.totalSpendingPaise || 0);
  document.getElementById('dashTotalCount').textContent = `${analytics.totalExpenseCount || 0} total transactions`;
  document.getElementById('dashAvgExpense').textContent = formatRupees(analytics.averageExpensePaise || 0);

  if (analytics.highestExpense) {
    document.getElementById('dashHighestExpense').textContent = formatRupees(analytics.highestExpense.amountPaise);
    document.getElementById('dashHighestTitle').textContent = `${analytics.highestExpense.title} (${analytics.highestExpense.category})`;
  } else {
    document.getElementById('dashHighestExpense').textContent = '₹0.00';
    document.getElementById('dashHighestTitle').textContent = '—';
  }

  document.getElementById('dashBudgetCount').textContent = state.budgets.length;
  document.getElementById('dashBudgetStatus').textContent = state.budgets.length > 0 ? 'Monthly tracking active' : 'No budget set';

  // Render recent 5 transactions
  const recent = [...state.expenses].sort((a, b) => b.date.localeCompare(a.date)).slice(0, 5);
  const tbody = document.getElementById('dashRecentExpensesBody');
  if (!tbody) return;

  if (recent.length === 0) {
    tbody.innerHTML = `<tr><td colspan="6" class="text-center py-4 text-muted">No expenses recorded yet. Click "Add Expense" to get started!</td></tr>`;
  } else {
    tbody.innerHTML = recent.map(e => `
      <tr>
        <td class="font-semibold">${escapeHtml(e.title)}</td>
        <td><span class="badge badge-indigo">${escapeHtml(e.category)}</span></td>
        <td>${escapeHtml(e.date)}</td>
        <td class="font-bold text-rose">${formatRupees(e.amountPaise)}</td>
        <td class="text-muted">${escapeHtml(e.note || '—')}</td>
        <td class="text-right">
          <div class="action-btn-group">
            <button class="btn btn-secondary btn-sm" onclick="openEditExpenseModal('${e.id}')">Edit</button>
            <button class="btn btn-danger-outline btn-sm" onclick="handleDeleteExpense('${e.id}')">Delete</button>
          </div>
        </td>
      </tr>
    `).join('');
  }

  // Render Dashboard Charts
  renderDashboardCharts();
}

function renderDashboardCharts() {
  if (typeof Chart === 'undefined') return;

  const catSpending = (state.analytics && state.analytics.categorySpending) || {};
  const monthlySpending = (state.analytics && state.analytics.monthlySpending) || {};

  const catLabels = Object.keys(catSpending);
  const catValues = Object.values(catSpending).map(v => v / 100);

  const monthLabels = Object.keys(monthlySpending);
  const monthValues = Object.values(monthlySpending).map(v => v / 100);

  // 1. Donut Chart
  const donutCanvas = document.getElementById('categoryDonutChart');
  const donutEmpty = document.getElementById('categoryChartEmpty');

  if (catLabels.length === 0) {
    if (donutEmpty) donutEmpty.classList.remove('hidden');
    if (state.charts.categoryDonut) state.charts.categoryDonut.destroy();
  } else {
    if (donutEmpty) donutEmpty.classList.add('hidden');
    if (state.charts.categoryDonut) state.charts.categoryDonut.destroy();

    const colors = ['#4f46e5', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#06b6d4', '#ec4899', '#84cc16'];
    state.charts.categoryDonut = new Chart(donutCanvas, {
      type: 'doughnut',
      data: {
        labels: catLabels,
        datasets: [{
          data: catValues,
          backgroundColor: colors.slice(0, catLabels.length),
          borderWidth: 2,
          borderColor: document.body.classList.contains('theme-dark') ? '#131b2e' : '#ffffff'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'bottom', labels: { boxWidth: 12, font: { family: 'Plus Jakarta Sans', size: 12 } } },
          tooltip: {
            callbacks: {
              label: (ctx) => ` ${ctx.label}: ₹${ctx.parsed.toFixed(2)}`
            }
          }
        },
        cutout: '68%'
      }
    });
  }

  // 2. Monthly Trend Chart
  const trendCanvas = document.getElementById('monthlyTrendChart');
  const trendEmpty = document.getElementById('monthlyChartEmpty');

  if (monthLabels.length === 0) {
    if (trendEmpty) trendEmpty.classList.remove('hidden');
    if (state.charts.monthlyTrend) state.charts.monthlyTrend.destroy();
  } else {
    if (trendEmpty) trendEmpty.classList.add('hidden');
    if (state.charts.monthlyTrend) state.charts.monthlyTrend.destroy();

    state.charts.monthlyTrend = new Chart(trendCanvas, {
      type: 'bar',
      data: {
        labels: monthLabels,
        datasets: [{
          label: 'Spending (₹)',
          data: monthValues,
          backgroundColor: 'rgba(79, 70, 229, 0.85)',
          borderRadius: 6,
          hoverBackgroundColor: '#4338ca'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          tooltip: {
            callbacks: {
              label: (ctx) => ` Spent: ₹${ctx.parsed.y.toFixed(2)}`
            }
          }
        },
        scales: {
          y: {
            beginAtZero: true,
            ticks: {
              callback: (val) => `₹${val}`
            },
            grid: { color: document.body.classList.contains('theme-dark') ? '#27354f' : '#f1f5f9' }
          },
          x: {
            grid: { display: false }
          }
        }
      }
    });
  }
}

// ============================================================
// EXPENSES TAB & FILTERING
// ============================================================

function updateCategoryFilterDropdown() {
  const select = document.getElementById('expenseCategoryFilter');
  if (!select) return;

  const currentVal = select.value;
  const categories = Array.from(new Set(state.expenses.map(e => e.category).filter(Boolean))).sort();

  select.innerHTML = `<option value="ALL">All Categories</option>` +
    categories.map(c => `<option value="${escapeHtml(c)}">${escapeHtml(c)}</option>`).join('');

  if (categories.includes(currentVal)) {
    select.value = currentVal;
  }
}

function applyExpenseFilters() {
  const { search, category, sortBy } = state.filters;
  let list = [...state.expenses];

  // 1. Search Filter
  if (search.trim()) {
    const q = search.toLowerCase();
    list = list.filter(e =>
      (e.title && e.title.toLowerCase().includes(q)) ||
      (e.note && e.note.toLowerCase().includes(q)) ||
      (e.category && e.category.toLowerCase().includes(q)) ||
      (e.id && e.id.toLowerCase().includes(q))
    );
  }

  // 2. Category Filter
  if (category && category !== 'ALL') {
    list = list.filter(e => e.category.toLowerCase() === category.toLowerCase());
  }

  // 3. Sorting
  list.sort((a, b) => {
    switch (sortBy) {
      case 'date-desc': return b.date.localeCompare(a.date);
      case 'date-asc': return a.date.localeCompare(b.date);
      case 'amount-desc': return b.amountPaise - a.amountPaise;
      case 'amount-asc': return a.amountPaise - b.amountPaise;
      case 'title-asc': return (a.title || '').localeCompare(b.title || '');
      default: return b.date.localeCompare(a.date);
    }
  });

  state.filteredExpenses = list;
  renderExpensesTable();
}

function renderExpensesTable() {
  const tbody = document.getElementById('expensesTableBody');
  const countEl = document.getElementById('expenseSummaryCount');
  const totalEl = document.getElementById('expenseSummaryTotal');
  if (!tbody) return;

  const list = state.filteredExpenses;
  const totalPaise = list.reduce((sum, e) => sum + (e.amountPaise || 0), 0);

  if (countEl) countEl.textContent = `Showing ${list.length} of ${state.expenses.length} expenses`;
  if (totalEl) totalEl.textContent = `Total: ${formatRupees(totalPaise)}`;

  if (list.length === 0) {
    tbody.innerHTML = `<tr><td colspan="7" class="text-center py-4 text-muted">No expenses match your search or filters.</td></tr>`;
    return;
  }

  tbody.innerHTML = list.map(e => `
    <tr>
      <td class="text-muted font-semibold text-xs">${escapeHtml(e.id)}</td>
      <td class="font-semibold">${escapeHtml(e.title)}</td>
      <td><span class="badge badge-indigo">${escapeHtml(e.category)}</span></td>
      <td>${escapeHtml(e.date)}</td>
      <td class="font-bold text-rose">${formatRupees(e.amountPaise)}</td>
      <td class="text-muted">${escapeHtml(e.note || '—')}</td>
      <td class="text-right">
        <div class="action-btn-group">
          <button class="btn btn-secondary btn-sm" onclick="openEditExpenseModal('${e.id}')">Edit</button>
          <button class="btn btn-danger-outline btn-sm" onclick="handleDeleteExpense('${e.id}')">Delete</button>
        </div>
      </td>
    </tr>
  `).join('');
}

// ============================================================
// ANALYTICS TAB RENDERING
// ============================================================

function renderAnalyticsTab() {
  const analytics = state.analytics;
  if (!analytics) return;

  // Peak Window
  const win = analytics.highestSpendingWindow7Days;
  const winTotalEl = document.getElementById('windowTotalPaise');
  const winDatesEl = document.getElementById('windowDates');

  if (win && win.found) {
    if (winTotalEl) winTotalEl.textContent = formatRupees(win.totalPaise);
    if (winDatesEl) winDatesEl.textContent = `Window: ${win.startDate} to ${win.endDate}`;
  } else {
    if (winTotalEl) winTotalEl.textContent = '₹0.00';
    if (winDatesEl) winDatesEl.textContent = 'Window: Insufficient data';
  }

  // Extremes
  const lowVal = document.getElementById('analyticsLowestVal');
  const lowTitle = document.getElementById('analyticsLowestTitle');
  const highVal = document.getElementById('analyticsHighestVal');
  const highTitle = document.getElementById('analyticsHighestTitle');

  if (analytics.lowestExpense) {
    if (lowVal) lowVal.textContent = formatRupees(analytics.lowestExpense.amountPaise);
    if (lowTitle) lowTitle.textContent = `${analytics.lowestExpense.title} (${analytics.lowestExpense.date})`;
  } else {
    if (lowVal) lowVal.textContent = '₹0.00';
    if (lowTitle) lowTitle.textContent = '—';
  }

  if (analytics.highestExpense) {
    if (highVal) highVal.textContent = formatRupees(analytics.highestExpense.amountPaise);
    if (highTitle) highTitle.textContent = `${analytics.highestExpense.title} (${analytics.highestExpense.date})`;
  } else {
    if (highVal) highVal.textContent = '₹0.00';
    if (highTitle) highTitle.textContent = '—';
  }

  // Category Analytics Table
  const tbody = document.getElementById('categoryAnalyticsTableBody');
  const catSpending = analytics.categorySpending || {};
  const totalSpendingPaise = analytics.totalSpendingPaise || 1;

  const entries = Object.entries(catSpending).sort((a, b) => b[1] - a[1]);

  if (entries.length === 0) {
    if (tbody) tbody.innerHTML = `<tr><td colspan="4" class="text-center py-4 text-muted">No category data available</td></tr>`;
  } else if (tbody) {
    tbody.innerHTML = entries.map(([category, amountPaise]) => {
      const pct = ((amountPaise / totalSpendingPaise) * 100).toFixed(1);
      return `
        <tr>
          <td class="font-semibold"><span class="badge badge-indigo">${escapeHtml(category)}</span></td>
          <td class="font-bold">${formatRupees(amountPaise)}</td>
          <td>${pct}%</td>
          <td>
            <div class="category-progress-track">
              <div class="category-progress-fill" style="width: ${pct}%"></div>
            </div>
          </td>
        </tr>
      `;
    }).join('');
  }

  // Daily Timeline Chart
  const timelineCanvas = document.getElementById('dailyTimelineChart');
  if (timelineCanvas && typeof Chart !== 'undefined') {
    const dailySpending = analytics.dailySpending || {};
    const dates = Object.keys(dailySpending).sort();
    const amounts = dates.map(d => dailySpending[d] / 100);

    if (state.charts.dailyTimeline) state.charts.dailyTimeline.destroy();

    state.charts.dailyTimeline = new Chart(timelineCanvas, {
      type: 'line',
      data: {
        labels: dates,
        datasets: [{
          label: 'Daily Spending (₹)',
          data: amounts,
          borderColor: '#4f46e5',
          backgroundColor: 'rgba(79, 70, 229, 0.1)',
          fill: true,
          tension: 0.3,
          pointRadius: 4,
          pointBackgroundColor: '#4f46e5'
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false },
          tooltip: {
            callbacks: {
              label: (ctx) => ` ₹${ctx.parsed.y.toFixed(2)}`
            }
          }
        },
        scales: {
          y: {
            beginAtZero: true,
            ticks: { callback: (v) => `₹${v}` },
            grid: { color: document.body.classList.contains('theme-dark') ? '#27354f' : '#f1f5f9' }
          },
          x: {
            grid: { display: false }
          }
        }
      }
    });
  }
}

// ============================================================
// BUDGETS TAB RENDERING
// ============================================================

function renderBudgetsTab() {
  const container = document.getElementById('budgetsGridContainer');
  if (!container) return;

  if (state.budgets.length === 0) {
    container.innerHTML = `
      <div class="card empty-card" style="grid-column: 1 / -1; text-align: center; padding: 3rem 1rem;">
        <p class="text-muted">No monthly budgets configured yet.</p>
        <button class="btn btn-primary btn-sm mt-2" onclick="openBudgetModal()">Set Your First Budget</button>
      </div>
    `;
    return;
  }

  container.innerHTML = state.budgets.map(b => {
    const pct = Math.min(Math.round(b.percentageUsed || 0), 100);
    let badgeClass = 'badge-emerald';
    let fillClass = 'budget-fill-within';
    let statusText = 'Within Budget';

    if (b.alert === 'EXCEEDED') {
      badgeClass = 'badge-rose';
      fillClass = 'budget-fill-exceeded';
      statusText = 'Budget Exceeded';
    } else if (b.alert === 'WARNING') {
      badgeClass = 'badge-amber';
      fillClass = 'budget-fill-warning';
      statusText = 'Warning (>80%)';
    }

    return `
      <div class="budget-card">
        <div>
          <div class="flex-between">
            <span class="badge badge-indigo">${escapeHtml(b.category)}</span>
            <span class="badge ${badgeClass}">${statusText}</span>
          </div>
          <div class="mt-2">
            <h3 class="text-xl font-bold">${formatRupees(b.actualSpendingPaise)} <span class="text-sm font-normal text-muted">of ${formatRupees(b.limitPaise)}</span></h3>
          </div>
          <div class="budget-progress-track">
            <div class="budget-progress-fill ${fillClass}" style="width: ${pct}%"></div>
          </div>
          <div class="flex-between text-xs text-muted">
            <span>${pct}% consumed</span>
            <span>${b.remainingPaise >= 0 ? `${formatRupees(b.remainingPaise)} left` : `${formatRupees(Math.abs(b.remainingPaise))} over limit`}</span>
          </div>
        </div>
        <div class="mt-4 text-right">
          <button class="btn btn-danger-outline btn-sm" onclick="handleDeleteBudget('${escapeHtml(b.category)}')">Remove Budget</button>
        </div>
      </div>
    `;
  }).join('');
}

// ============================================================
// SMART SPLITTER (GROUPS) TAB RENDERING
// ============================================================

function renderGroupsSidebar() {
  const container = document.getElementById('groupsListContainer');
  if (!container) return;

  if (state.groups.length === 0) {
    container.innerHTML = `<div class="p-3 text-muted text-center text-sm">No groups yet. Click "Create Group" to start splitting!</div>`;
    return;
  }

  container.innerHTML = state.groups.map(g => {
    const isActive = g.id === state.activeGroupId;
    return `
      <button class="group-item-btn ${isActive ? 'active' : ''}" onclick="selectGroup('${g.id}')">
        <div class="font-bold">${escapeHtml(g.name)}</div>
        <div class="text-xs text-muted mt-1">${g.memberCount || 0} members • Code: ${escapeHtml(g.groupCode)}</div>
      </button>
    `;
  }).join('');
}

async function selectGroup(groupId) {
  state.activeGroupId = groupId;
  renderGroupsSidebar();

  try {
    const details = await apiRequest(`/groups/${groupId}`);
    state.activeGroupDetails = details;
    renderGroupDetails(details);
  } catch (err) {
    showToast('Failed to load group details', 'error');
  }
}

function renderGroupDetails(details) {
  document.getElementById('groupCodeBadge').textContent = `CODE: ${details.groupCode || '—'}`;
  document.getElementById('groupNameTitle').textContent = details.name;

  const memberNames = (details.members || []).map(m => m.name).join(', ');
  document.getElementById('groupMembersList').textContent = `Members: ${memberNames || 'None'}`;

  const addGeBtn = document.getElementById('openAddGroupExpenseBtn');
  if (addGeBtn) addGeBtn.classList.remove('hidden');

  // 1. Balances List
  const balancesContainer = document.getElementById('groupBalancesList');
  if (balancesContainer) {
    const balances = details.balances || [];
    if (balances.length === 0) {
      balancesContainer.innerHTML = `<p class="text-muted p-3">No balances calculated yet.</p>`;
    } else {
      balancesContainer.innerHTML = balances.map(b => {
        const isPos = b.netBalancePaise > 0;
        const isZero = b.netBalancePaise === 0;
        const colorClass = isZero ? 'text-muted' : (isPos ? 'text-emerald' : 'text-rose');
        const sign = isPos ? '+' : '';
        return `
          <div class="balance-item">
            <span class="font-semibold">${escapeHtml(b.userName)}</span>
            <span class="font-bold ${colorClass}">${sign}${formatRupees(b.netBalancePaise)}</span>
          </div>
        `;
      }).join('');
    }
  }

  // 2. Settlements Plan List
  const settlementsContainer = document.getElementById('groupSettlementsList');
  if (settlementsContainer) {
    const settlements = details.settlements || [];
    if (settlements.length === 0) {
      settlementsContainer.innerHTML = `<div class="p-3 text-emerald font-semibold">All balances are fully settled! No pending payments.</div>`;
    } else {
      settlementsContainer.innerHTML = settlements.map(s => `
        <div class="settlement-item">
          <div class="settlement-arrow">
            <span class="font-bold text-main">${escapeHtml(s.fromUserName)}</span>
            <span>pays</span>
            <span class="font-bold text-main">${escapeHtml(s.toUserName)}</span>
          </div>
          <span class="font-bold text-rose">${formatRupees(s.amountPaise)}</span>
        </div>
      `).join('');
    }
  }

  // 3. Group Expenses History
  const expensesTbody = document.getElementById('groupExpensesTableBody');
  if (expensesTbody) {
    const expenses = details.expenses || [];
    if (expenses.length === 0) {
      expensesTbody.innerHTML = `<tr><td colspan="6" class="text-center py-4 text-muted">No group expenses recorded yet. Click "Add Group Expense" to split a bill!</td></tr>`;
    } else {
      expensesTbody.innerHTML = expenses.map(ge => {
        const splitText = (ge.splits || []).map(sp => `${sp.userName}: ${formatRupees(sp.sharePaise)}`).join(', ');
        return `
          <tr>
            <td class="font-semibold">${escapeHtml(ge.description)}</td>
            <td><span class="badge badge-indigo">${escapeHtml(ge.category)}</span></td>
            <td><span class="badge badge-emerald">${escapeHtml(ge.paidByName)}</span></td>
            <td class="font-bold text-rose">${formatRupees(ge.amountPaise)}</td>
            <td>${escapeHtml(ge.date)}</td>
            <td class="text-xs text-muted">${escapeHtml(splitText)}</td>
          </tr>
        `;
      }).join('');
    }
  }
}

// ============================================================
// MODAL MANAGEMENT & FORM HANDLERS
// ============================================================

function openAddExpenseModal() {
  document.getElementById('expenseModalTitle').textContent = 'Add Expense';
  document.getElementById('formExpenseId').value = '';
  document.getElementById('formTitle').value = '';
  document.getElementById('formCategory').value = 'Food';
  document.getElementById('formAmount').value = '';
  document.getElementById('formDate').value = getTodayString();
  document.getElementById('formNote').value = '';
  document.getElementById('expenseModal').classList.remove('hidden');
}

function openEditExpenseModal(id) {
  const expense = state.expenses.find(e => e.id === id);
  if (!expense) return;

  document.getElementById('expenseModalTitle').textContent = 'Edit Expense';
  document.getElementById('formExpenseId').value = expense.id;
  document.getElementById('formTitle').value = expense.title;
  document.getElementById('formCategory').value = expense.category;
  document.getElementById('formAmount').value = paiseToRupees(expense.amountPaise);
  document.getElementById('formDate').value = expense.date;
  document.getElementById('formNote').value = expense.note || '';
  document.getElementById('expenseModal').classList.remove('hidden');
}

async function handleExpenseSubmit(e) {
  e.preventDefault();
  const id = document.getElementById('formExpenseId').value;
  const title = document.getElementById('formTitle').value.trim();
  const category = document.getElementById('formCategory').value.trim();
  const amountRupees = parseFloat(document.getElementById('formAmount').value);
  const date = document.getElementById('formDate').value;
  const note = document.getElementById('formNote').value.trim();

  if (!title || !category || isNaN(amountRupees) || amountRupees <= 0 || !date) {
    showToast('Please fill all required fields properly.', 'error');
    return;
  }

  const payload = {
    title,
    category,
    amountPaise: rupeesToPaise(amountRupees),
    date,
    note
  };

  try {
    if (id) {
      await apiRequest(`/expenses/${id}`, { method: 'PUT', body: JSON.stringify(payload) });
      showToast('Expense updated successfully!', 'success');
    } else {
      await apiRequest('/expenses', { method: 'POST', body: JSON.stringify(payload) });
      showToast('Expense created successfully!', 'success');
    }
    closeAllModals();
    await syncAllData();
  } catch (err) {
    showToast(`Failed to save expense: ${err.message}`, 'error');
  }
}

async function handleDeleteExpense(id) {
  if (!confirm(`Are you sure you want to delete expense ${id}?`)) return;

  try {
    await apiRequest(`/expenses/${id}`, { method: 'DELETE' });
    showToast('Expense deleted successfully!', 'success');
    await syncAllData();
  } catch (err) {
    showToast(`Failed to delete expense: ${err.message}`, 'error');
  }
}

// Budget Modal & Handlers
function openBudgetModal() {
  document.getElementById('budgetFormCategory').value = 'ALL';
  document.getElementById('budgetFormLimit').value = '10000';
  document.getElementById('budgetModal').classList.remove('hidden');
}

async function handleBudgetSubmit(e) {
  e.preventDefault();
  const category = document.getElementById('budgetFormCategory').value.trim();
  const limitRupees = parseFloat(document.getElementById('budgetFormLimit').value);

  if (!category || isNaN(limitRupees) || limitRupees <= 0) {
    showToast('Please enter a valid category and budget limit.', 'error');
    return;
  }

  const payload = {
    category,
    limitPaise: rupeesToPaise(limitRupees),
    period: 2 // MONTHLY
  };

  try {
    await apiRequest('/budgets', { method: 'POST', body: JSON.stringify(payload) });
    showToast(`Budget for ${category} saved!`, 'success');
    closeAllModals();
    await syncAllData();
  } catch (err) {
    showToast(`Failed to set budget: ${err.message}`, 'error');
  }
}

async function handleDeleteBudget(category) {
  if (!confirm(`Remove budget for category "${category}"?`)) return;

  try {
    await apiRequest(`/budgets/${encodeURIComponent(category)}`, { method: 'DELETE' });
    showToast(`Budget for ${category} removed`, 'success');
    await syncAllData();
  } catch (err) {
    showToast(`Failed to remove budget: ${err.message}`, 'error');
  }
}

// Group Modal & Handlers
function openCreateGroupModal() {
  document.getElementById('groupFormName').value = '';
  document.getElementById('groupFormMembers').value = '';
  document.getElementById('groupModal').classList.remove('hidden');
}

async function handleGroupSubmit(e) {
  e.preventDefault();
  const name = document.getElementById('groupFormName').value.trim();
  const membersRaw = document.getElementById('groupFormMembers').value.trim();

  if (!name) {
    showToast('Group name is required.', 'error');
    return;
  }

  const members = membersRaw
    .split(',')
    .map(m => m.trim())
    .filter(Boolean);

  try {
    const res = await apiRequest('/groups', {
      method: 'POST',
      body: JSON.stringify({ name, members })
    });
    showToast(`Group "${name}" created!`, 'success');
    closeAllModals();
    await syncAllData();
    if (res && res.id) {
      selectGroup(res.id);
    }
  } catch (err) {
    showToast(`Failed to create group: ${err.message}`, 'error');
  }
}

// Group Expense Modal & Handlers
function openAddGroupExpenseModal() {
  if (!state.activeGroupDetails) {
    showToast('Select a group first!', 'error');
    return;
  }

  const members = state.activeGroupDetails.members || [];
  const paidBySelect = document.getElementById('geFormPaidBy');
  paidBySelect.innerHTML = members.map(m => `<option value="${m.id}">${escapeHtml(m.name)}</option>`).join('');

  document.getElementById('groupExpenseFormGroupId').value = state.activeGroupId;
  document.getElementById('geFormTitle').value = '';
  document.getElementById('geFormCategory').value = 'Food';
  document.getElementById('geFormAmount').value = '';
  document.getElementById('geFormDate').value = getTodayString();
  document.getElementById('groupExpenseModal').classList.remove('hidden');
}

async function handleGroupExpenseSubmit(e) {
  e.preventDefault();
  const groupId = document.getElementById('groupExpenseFormGroupId').value;
  const title = document.getElementById('geFormTitle').value.trim();
  const category = document.getElementById('geFormCategory').value.trim() || 'Other';
  const amountRupees = parseFloat(document.getElementById('geFormAmount').value);
  const paidByUserId = document.getElementById('geFormPaidBy').value;
  const date = document.getElementById('geFormDate').value;

  if (!groupId || !title || isNaN(amountRupees) || amountRupees <= 0 || !paidByUserId || !date) {
    showToast('Please fill all required fields.', 'error');
    return;
  }

  const payload = {
    title,
    category,
    amountPaise: rupeesToPaise(amountRupees),
    paidByUserId,
    date
  };

  try {
    await apiRequest(`/groups/${groupId}/expenses`, {
      method: 'POST',
      body: JSON.stringify(payload)
    });
    showToast('Group expense recorded and split calculated!', 'success');
    closeAllModals();
    await selectGroup(groupId);
  } catch (err) {
    showToast(`Failed to add group expense: ${err.message}`, 'error');
  }
}

function closeAllModals() {
  document.querySelectorAll('.modal-backdrop').forEach(m => m.classList.add('hidden'));
}

// ============================================================
// EXPORT DATA
// ============================================================

function exportExpensesToCsv() {
  if (state.expenses.length === 0) {
    showToast('No expenses available to export.', 'error');
    return;
  }

  const headers = ['ID', 'User ID', 'Title', 'Category', 'Amount (Paise)', 'Amount (INR)', 'Date', 'Note'];
  const rows = state.expenses.map(e => [
    `"${(e.id || '').replace(/"/g, '""')}"`,
    `"${(e.userId || '').replace(/"/g, '""')}"`,
    `"${(e.title || '').replace(/"/g, '""')}"`,
    `"${(e.category || '').replace(/"/g, '""')}"`,
    e.amountPaise,
    (e.amountPaise / 100).toFixed(2),
    `"${(e.date || '').replace(/"/g, '""')}"`,
    `"${(e.note || '').replace(/"/g, '""')}"`
  ]);

  const csvContent = 'data:text/csv;charset=utf-8,' + [headers.join(','), ...rows.map(r => r.join(','))].join('\n');
  const encodedUri = encodeURI(csvContent);
  const link = document.createElement('a');
  link.setAttribute('href', encodedUri);
  link.setAttribute('download', `personal_expenses_${getTodayString()}.csv`);
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  showToast('Expenses exported to CSV!', 'success');
}

// ============================================================
// NAVIGATION & THEME
// ============================================================

function switchTab(tabId) {
  state.currentTab = tabId;

  document.querySelectorAll('.nav-tab').forEach(tab => {
    tab.classList.toggle('active', tab.getAttribute('data-tab') === tabId);
  });

  document.querySelectorAll('.tab-pane').forEach(pane => {
    pane.classList.toggle('active', pane.id === `tab-${tabId}`);
  });

  // Re-render chart layouts on tab switch
  if (tabId === 'dashboard') {
    renderDashboardCharts();
  } else if (tabId === 'analytics') {
    renderAnalyticsTab();
  }
}

function initTheme() {
  const saved = localStorage.getItem('theme') || 'light';
  applyTheme(saved);

  const toggleBtn = document.getElementById('themeToggleBtn');
  if (toggleBtn) {
    toggleBtn.addEventListener('click', () => {
      const isDark = document.body.classList.contains('theme-dark');
      applyTheme(isDark ? 'light' : 'dark');
    });
  }
}

function applyTheme(theme) {
  if (theme === 'dark') {
    document.body.classList.remove('theme-light');
    document.body.classList.add('theme-dark');
    document.getElementById('themeIconSun').classList.add('hidden');
    document.getElementById('themeIconMoon').classList.remove('hidden');
  } else {
    document.body.classList.remove('theme-dark');
    document.body.classList.add('theme-light');
    document.getElementById('themeIconMoon').classList.add('hidden');
    document.getElementById('themeIconSun').classList.remove('hidden');
  }
  localStorage.setItem('theme', theme);
  renderDashboardCharts();
  if (state.currentTab === 'analytics') renderAnalyticsTab();
}

// ============================================================
// EVENT LISTENERS INITIALIZATION
// ============================================================

document.addEventListener('DOMContentLoaded', () => {
  initTheme();

  // Tab navigation
  document.querySelectorAll('.nav-tab').forEach(tab => {
    tab.addEventListener('click', () => switchTab(tab.getAttribute('data-tab')));
  });

  document.querySelectorAll('.view-all-expenses-btn').forEach(btn => {
    btn.addEventListener('click', () => switchTab('expenses'));
  });

  // Add Expense Triggers
  const openAddBtn = document.getElementById('openAddExpenseBtn');
  if (openAddBtn) openAddBtn.addEventListener('click', openAddExpenseModal);

  document.querySelectorAll('.trigger-add-expense').forEach(b => {
    b.addEventListener('click', openAddExpenseModal);
  });

  // Budget Modal Triggers
  const openBudgetBtn = document.getElementById('openSetBudgetBtn');
  if (openBudgetBtn) openBudgetBtn.addEventListener('click', openBudgetModal);

  // Group Modal Triggers
  const openGroupBtn = document.getElementById('openCreateGroupBtn');
  if (openGroupBtn) openGroupBtn.addEventListener('click', openCreateGroupModal);

  const openGroupExpenseBtn = document.getElementById('openAddGroupExpenseBtn');
  if (openGroupExpenseBtn) openGroupExpenseBtn.addEventListener('click', openAddGroupExpenseModal);

  // Close modals
  document.querySelectorAll('[data-close-modal]').forEach(b => {
    b.addEventListener('click', closeAllModals);
  });

  // Form Submits
  const expenseForm = document.getElementById('expenseForm');
  if (expenseForm) expenseForm.addEventListener('submit', handleExpenseSubmit);

  const budgetForm = document.getElementById('budgetForm');
  if (budgetForm) budgetForm.addEventListener('submit', handleBudgetSubmit);

  const groupForm = document.getElementById('groupForm');
  if (groupForm) groupForm.addEventListener('submit', handleGroupSubmit);

  const groupExpenseForm = document.getElementById('groupExpenseForm');
  if (groupExpenseForm) groupExpenseForm.addEventListener('submit', handleGroupExpenseSubmit);

  // Search & Filter Events
  const searchInput = document.getElementById('expenseSearchInput');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      state.filters.search = e.target.value;
      applyExpenseFilters();
    });
  }

  const categoryFilter = document.getElementById('expenseCategoryFilter');
  if (categoryFilter) {
    categoryFilter.addEventListener('change', (e) => {
      state.filters.category = e.target.value;
      applyExpenseFilters();
    });
  }

  const sortSelect = document.getElementById('expenseSortSelect');
  if (sortSelect) {
    sortSelect.addEventListener('change', (e) => {
      state.filters.sortBy = e.target.value;
      applyExpenseFilters();
    });
  }

  // Export CSV
  const exportBtn = document.getElementById('exportCsvBtn');
  if (exportBtn) exportBtn.addEventListener('click', exportExpensesToCsv);

  // Initial Sync
  syncAllData();
});
